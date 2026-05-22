// backend/compiler.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#define MAXTOK 256
#define MAXSYM 128
#define MAXTMP 128

// ----------------- Globals / Structures -----------------
typedef struct {
    char name[64];
    char type[16]; // "int" or "float"
    int id;        // 1-based id: id1, id2...
} Sym;

Sym symtab[MAXSYM];
int symcount = 0;

char tokens_out[4096];
char normalized[1024];
char parse_tree[4096];
char varmap[1024];
char semantic_tree[8192];
char var_types[1024];
char tac[4096];
char opt_tac[4096];
char opt_info[1024];
char asm_code[4096];

int tmp_counter = 1;

// Error reporting
int had_error = 0;
char error_phase[64] = "";
char error_msg[1024] = "";

// ----------------- Helpers -----------------
int sym_lookup(const char* name) {
    for (int i=0;i<symcount;i++)
        if (strcmp(symtab[i].name, name)==0) return i;
    return -1;
}

int sym_add(const char* name) {
    int idx = sym_lookup(name);
    if (idx>=0) return idx;
    if (symcount >= MAXSYM) return -1;
    strncpy(symtab[symcount].name, name, 63);
    strcpy(symtab[symcount].type, "int"); // default int
    symtab[symcount].id = symcount+1;
    symcount++;
    return symcount-1;
}

void append(char* dest, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    char buf[1024];
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    strncat(dest, buf, 4095 - strlen(dest));
}

void set_error(const char* phase, const char* fmt, ...) {
    if (had_error) return; // keep first error
    had_error = 1;
    strncpy(error_phase, phase, sizeof(error_phase)-1);
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(error_msg, sizeof(error_msg), fmt, ap);
    va_end(ap);
}

// ------------------ JSON escape helper ------------------
void json_escape(char *dest, const char *src, int max) {
    int i = 0, j = 0;
    while(src[i] && j < max-1) {
        unsigned char c = (unsigned char)src[i++];
        switch(c) {
            case '"': dest[j++]='\\'; dest[j++]='"'; break;
            case '\\': dest[j++]='\\'; dest[j++]='\\'; break;
            case '\n': dest[j++]='\\'; dest[j++]='n'; break;
            case '\r': /* skip CR */ break;
            case '\t': dest[j++]='\\'; dest[j++]='t'; break;
            default:
                if (c < 32) {
                    if (j + 6 < max-1) {
                        int written = snprintf(dest+j, max-j, "\\u%04x", c);
                        j += written;
                    }
                } else {
                    dest[j++] = c;
                }
                break;
        }
    }
    dest[j] = 0;
}

// ---------------- Lexical ----------------
typedef enum {TOK_ID, TOK_NUM, TOK_ASSIGN, TOK_PLUS, TOK_MINUS, TOK_MUL, TOK_DIV, TOK_LP, TOK_RP, TOK_END, TOK_INVALID} TokType;
typedef struct { TokType type; char lexeme[64]; int pos; } Token;

Token toks[MAXTOK];
int tokcount = 0;

void clear_tokens() { tokcount = 0; tokens_out[0]=0; normalized[0]=0; }

void scan_lexical(const char* s) {
    clear_tokens();
    int i=0, n=strlen(s);
    char buf[64];

    while (i<n) {
        if (isspace((unsigned char)s[i])) { i++; continue; }

        // identifier
        if (isalpha((unsigned char)s[i])) {
            int j=0;
            int start = i;
            while (i<n && (isalnum((unsigned char)s[i])|| s[i]=='_')) buf[j++]=s[i++];
            buf[j]=0;
            Token t; t.type = TOK_ID; strncpy(t.lexeme, buf, 63); t.pos = start;
            toks[tokcount++] = t;
            // append token in new format: <idN,name> where N will be assigned after sym_add
            int si = sym_add(buf); // ensures added
            append(tokens_out, "<id%d,%s> ", si+1, buf);
            continue;
        }

        // number
        if (isdigit((unsigned char)s[i]) || (s[i]=='.' && i+1<n && isdigit((unsigned char)s[i+1]))) {
            int j=0; int start = i; int dot = 0;
            while (i<n && (isdigit((unsigned char)s[i])|| s[i]=='.')) {
                if (s[i]=='.') dot = 1;
                buf[j++]=s[i++];
            }
            buf[j]=0;
            Token t; t.type=TOK_NUM; strncpy(t.lexeme, buf, 63); t.pos = start;
            toks[tokcount++]=t;
            append(tokens_out, "<num,%s> ", buf);
            continue;
        }

        // operators and punctuation
        int start = i;
        char c = s[i++];

        // detect double-operator like '**' (invalid)
        if (c == '*' && i < n && s[i] == '*') {
            // invalid operator **
            set_error("lexical", "Invalid operator '**' at position %d", start);
            Token t; t.type = TOK_INVALID; t.lexeme[0]=c; t.lexeme[1]=s[i]; t.lexeme[2]=0; t.pos = start;
            toks[tokcount++] = t;
            i++; // consume second '*'
            append(tokens_out, "<%s> ", "**");
            continue;
        }

        Token t; t.lexeme[0]=c; t.lexeme[1]=0; t.pos = start;
        if (c=='=') t.type=TOK_ASSIGN;
        else if (c=='+') t.type=TOK_PLUS;
        else if (c=='-') t.type=TOK_MINUS;
        else if (c=='*') t.type=TOK_MUL;
        else if (c=='/') t.type=TOK_DIV;
        else if (c=='(') t.type=TOK_LP;
        else if (c==')') t.type=TOK_RP;
        else { t.type = TOK_INVALID;
               set_error("lexical", "Invalid character '%c' at position %d", c, start);
               append(tokens_out, "<%c> ", c);
        }
        toks[tokcount++] = t;
    }
    Token end; end.type = TOK_END; strcpy(end.lexeme,"<END>"); end.pos = n;
    toks[tokcount++] = end;

    // normalized (id numbers)
    char normbuf[1024] = {0};
    for (int i=0;i<tokcount;i++) {
        Token *t=&toks[i];
        if (t->type==TOK_ID) {
            int idx = sym_lookup(t->lexeme);
            append(normbuf,"id%d ", idx>=0? idx+1:0);
        } else append(normbuf,"%s ", t->lexeme);
    }
    int L=strlen(normbuf); while(L>0 && isspace((unsigned char)normbuf[L-1])) {normbuf[L-1]=0;L--;}
    strncpy(normalized, normbuf, sizeof(normalized)-1);

    // symbol table text (initial types default int)
    char sm[512] = "ID    Variable   Type\n";
    for(int i=0;i<symcount;i++)
        append(sm,"id%d    %-8s   %s\n",i+1,symtab[i].name,symtab[i].type);
    strncpy(varmap, sm, sizeof(varmap)-1);
}

// ---------------- Parser & AST ----------------
typedef enum {NODE_ID, NODE_NUM, NODE_BINOP, NODE_ASSIGN} NodeType;
typedef struct Node {
    NodeType kind;
    char text[64];
    struct Node *left, *right;
} Node;

Node* make_node(NodeType k, const char* txt, Node* l, Node* r) {
    Node* n = (Node*)malloc(sizeof(Node));
    n->kind = k;
    strncpy(n->text, txt?txt:"", 63);
    n->left = l; n->right = r;
    return n;
}

int curtok = 0;
Token* curToken(){ return &toks[curtok]; }
void advance(){ if (curtok < tokcount-1) curtok++; }

// Forward
Node* parse_expr();

Node* parse_factor() {
    Token *t = curToken();
    if (t->type == TOK_ID) {
        Node* n = make_node(NODE_ID, t->lexeme, NULL, NULL);
        advance();
        return n;
    } else if (t->type == TOK_NUM) {
        Node* n = make_node(NODE_NUM, t->lexeme, NULL, NULL);
        advance();
        return n;
    } else if (t->type == TOK_LP) {
        advance();
        Node* e = parse_expr();
        if (curToken()->type == TOK_RP) advance();
        else set_error("syntax", "Expected ')' but not found");
        return e;
    } else {
        set_error("syntax", "Unexpected token '%s' at parse_factor", t->lexeme);
        advance();
        return make_node(NODE_NUM, "0", NULL, NULL);
    }
}

Node* parse_term() {
    Node* node = parse_factor();
    while (curToken()->type==TOK_MUL || curToken()->type==TOK_DIV) {
        if (had_error && strcmp(error_phase,"lexical")==0) break;
        char op = curToken()->lexeme[0];
        advance();
        Node* right = parse_factor();
        char opstr[3] = {op,0,0};
        node = make_node(NODE_BINOP, opstr, node, right);
    }
    return node;
}

Node* parse_expr() {
    Node* node = parse_term();
    while (curToken()->type==TOK_PLUS || curToken()->type==TOK_MINUS) {
        char op = curToken()->lexeme[0];
        advance();
        Node* right = parse_term();
        char opstr[3] = {op,0,0};
        node = make_node(NODE_BINOP, opstr, node, right);
    }
    return node;
}

Node* parse_assignment() {
    if (curToken()->type==TOK_ID && toks[curtok+1].type==TOK_ASSIGN) {
        char tmpid[64]; strcpy(tmpid, curToken()->lexeme);
        advance(); // id
        advance(); // =
        Node* e = parse_expr();
        Node* lhs = make_node(NODE_ID, tmpid, NULL, NULL);
        return make_node(NODE_ASSIGN, "=", lhs, e);
    } else {
        return parse_expr();
    }
}

// ---------- Pretty parse-tree (branch style) ----------
void print_parse_ascii(Node* n, char* prefix, int isLeft, char* out){
    if(!n) return;
    append(out,"%s",prefix);
    append(out,"%s",isLeft?"├── ":"└── ");
    if(n->kind==NODE_ASSIGN) append(out,"ASSIGN\n");
    else if(n->kind==NODE_BINOP) {
        if (n->text[0]=='+') append(out,"ADD\n");
        else if (n->text[0]=='-') append(out,"SUB\n");
        else if (n->text[0]=='*') append(out,"MUL\n");
        else if (n->text[0]=='/') append(out,"DIV\n");
        else append(out,"OP(%s)\n", n->text);
    } else append(out,"%s\n", n->text);
    char newPrefix[256];
    snprintf(newPrefix,sizeof(newPrefix),"%s%s",prefix,isLeft?"│   ":"    ");
    if(n->left && n->right){ print_parse_ascii(n->left,newPrefix,1,out); print_parse_ascii(n->right,newPrefix,0,out);}
    else if(n->left) print_parse_ascii(n->left,newPrefix,0,out);
    else if(n->right) print_parse_ascii(n->right,newPrefix,0,out);
}

// ----------------- Semantic Analysis -----------------
int subtree_contains_float(Node* n) {
    if(!n) return 0;
    if(n->kind==NODE_NUM) return strchr(n->text,'.') ? 1:0;
    int left = subtree_contains_float(n->left);
    int right = subtree_contains_float(n->right);
    return left || right;
}

void mark_ids_float_recursive(Node* n) {
    if(!n) return;
    if (subtree_contains_float(n)) {
        if (n->kind==NODE_ID) {
            int idx=sym_lookup(n->text);
            if (idx>=0) strcpy(symtab[idx].type,"float");
        } else {
            if (n->left) mark_ids_float_recursive(n->left);
            if (n->right) mark_ids_float_recursive(n->right);
            if (n->left && n->left->kind==NODE_ID) { int idx=sym_lookup(n->left->text); if(idx>=0) strcpy(symtab[idx].type,"float"); }
            if (n->right && n->right->kind==NODE_ID) { int idx=sym_lookup(n->right->text); if(idx>=0) strcpy(symtab[idx].type,"float"); }
        }
    } else {
        if (n->left) mark_ids_float_recursive(n->left);
        if (n->right) mark_ids_float_recursive(n->right);
    }
}

void print_semantic_ascii(Node* n, char* prefix, int isLeft, char* out) {
    if(!n) return;
    append(out,"%s",prefix);
    append(out,"%s",isLeft?"├── ":"└── ");
    if (n->kind==NODE_ASSIGN) append(out,"ASSIGN\n");
    else if (n->kind==NODE_BINOP) {
        if (n->text[0]=='+') append(out,"ADD\n");
        else if (n->text[0]=='-') append(out,"SUB\n");
        else if (n->text[0]=='*') append(out,"MUL\n");
        else if (n->text[0]=='/') append(out,"DIV\n");
        else append(out,"OP(%s)\n", n->text);
    } else if (n->kind==NODE_ID) {
        int idx = sym_lookup(n->text);
        const char *typ = (idx>=0)? symtab[idx].type : "id";
        append(out,"VAR(%s : %s)\n", n->text, typ);
    } else if (n->kind==NODE_NUM) {
        if (strchr(n->text,'.')) append(out,"FLOAT(%s)\n", n->text);
        else append(out,"INT(%s)\n", n->text);
    } else {
        append(out,"%s\n", n->text);
    }
    char newPrefix[256];
    snprintf(newPrefix,sizeof(newPrefix),"%s%s",prefix,isLeft?"│   ":"    ");
    if(n->left && n->right){ print_semantic_ascii(n->left,newPrefix,1,out); print_semantic_ascii(n->right,newPrefix,0,out);}
    else if(n->left) print_semantic_ascii(n->left,newPrefix,0,out);
    else if(n->right) print_semantic_ascii(n->right,newPrefix,0,out);
}
// -------- helper: detect numeric literal ----------
int is_number(const char* s){
    if(!s || !*s) return 0;
    if(*s=='-') s++;
    int dot = 0;
    while(*s){
        if(*s=='.'){
            if(dot) return 0;
            dot = 1;
        }
        else if(!isdigit(*s)) return 0;
        s++;
    }
    return 1;
}

// ---------------- TAC generation (CORRECT, textbook style) ----------------
// ---------------- TAC generation (CORRECT, textbook style) ----------------
void gen_tac_rec(Node* n, char* outbuf){
    if(!n) return;

    if(n->kind == NODE_NUM){
        // int literal → separate inttofloat temp
        if(strchr(n->text,'.') == NULL){
            char t[32];
            sprintf(t,"t%d", tmp_counter++);
            append(tac,"%s = inttofloat(%s)\n", t, n->text);
            append(outbuf,"%s", t);
        }else{
            append(outbuf,"%s", n->text);
        }
    }
    else if(n->kind == NODE_ID){
        int idx = sym_lookup(n->text);
        append(outbuf,"id%d", idx+1);
    }
    else if(n->kind == NODE_BINOP){
        char l[64]={0}, r[64]={0};
        gen_tac_rec(n->left, l);
        gen_tac_rec(n->right, r);

        char t[32];
        sprintf(t,"t%d", tmp_counter++);
        append(tac,"%s = %s %s %s\n", t, l, n->text, r);
        append(outbuf,"%s", t);
    }
    else if(n->kind == NODE_ASSIGN){
        char rhs[64]={0};
        gen_tac_rec(n->right, rhs);
        int idx = sym_lookup(n->left->text);
        append(tac,"id%d = %s\n", idx+1, rhs);
    }
}



// ----------------- Optimized TAC (remove unnecessary temps) -----------------
void optimize_tac(){
    char buf[4096]; buf[0]=0;
    char line[256];
    char *p = tac;

    // store temp numeric values
    char temp_vals[128][2][64]; // temp name -> value
    int temp_count = 0;

    while(*p){
        char *nl = strchr(p,'\n');
        int len = nl ? (int)(nl-p) : (int)strlen(p);
        if(len>255) len=255;
        strncpy(line,p,len); line[len]=0;
        p = nl ? nl+1 : p+len;

        char lhs[64], r1[64], op[8], r2[64];

        // -------- binary op --------
        if(sscanf(line,"%63s = %63s %7s %63s",lhs,r1,op,r2)==4){
            // replace temp with stored numeric values
            for(int i=0;i<temp_count;i++){
                if(strcmp(r1,temp_vals[i][0])==0) strncpy(r1,temp_vals[i][1],sizeof(r1)-1);
                if(strcmp(r2,temp_vals[i][0])==0) strncpy(r2,temp_vals[i][1],sizeof(r2)-1);
            }

            append(buf,"%s = %s %s %s\n",lhs,r1,op,r2);
        }
        // -------- assignment --------
        else if(sscanf(line,"%63s = %63s",lhs,r1)==2){
            // drop inttofloat temp assignments
            if(strncmp(r1,"inttofloat(",11)==0){
                char val[64]; sscanf(r1,"inttofloat(%63[^)])",val);
                // store temp value
                strncpy(temp_vals[temp_count][0],lhs,sizeof(temp_vals[0][0])-1);
                snprintf(temp_vals[temp_count][1],sizeof(temp_vals[0][1]),"%s.0",val);
                temp_count++;
                continue;
            }
            // replace temp if it's numeric
            for(int i=0;i<temp_count;i++){
                if(strcmp(r1,temp_vals[i][0])==0) strncpy(r1,temp_vals[i][1],sizeof(r1)-1);
            }
            append(buf,"%s = %s\n",lhs,r1);
        }
    }

    strncpy(opt_tac,buf,sizeof(opt_tac)-1);
}


// ---------------- Assembly Code Generation with Register Reuse ----------------
typedef struct {
    char temp[16];
    char reg[4];
} TempMap;

TempMap temp_map[128];
int temp_map_count = 0;

// helper: find or assign register for a temp
const char* get_reg(const char* temp, int* reg_counter) {
    if(temp[0] != 't') return NULL; // not a temp

    // check if already mapped
    for(int i=0;i<temp_map_count;i++){
        if(strcmp(temp_map[i].temp,temp)==0) return temp_map[i].reg;
    }

    // assign new register
    char reg[4];
    sprintf(reg,"R%d",(*reg_counter)++);
    strncpy(temp_map[temp_map_count].temp,temp,sizeof(temp_map[temp_map_count].temp)-1);
    strncpy(temp_map[temp_map_count].reg,reg,sizeof(temp_map[temp_map_count].reg)-1);
    temp_map_count++;
    return temp_map[temp_map_count-1].reg;
}

void generate_assembly_optimized() {
    asm_code[0] = 0;
    temp_map_count = 0;
    int reg_counter = 1; // start from R1

    char *p = opt_tac;
    char line[256];

    while(*p){
        char *nl = strchr(p,'\n');
        int len = nl ? (int)(nl-p) : (int)strlen(p);
        if(len>255) len=255;
        strncpy(line,p,len); line[len]=0;
        p = nl ? nl+1 : p+len;

        char lhs[64], r1[64], op[8], r2[64];

        // ---------- binary operation ----------
        if(sscanf(line,"%63s = %63s %7s %63s", lhs,r1,op,r2)==4){
            const char *reg1, *reg2, *regout;

            // operand 1
            if(r1[0]=='t') reg1 = get_reg(r1,&reg_counter);
            else {
                char tmp[4]; sprintf(tmp,"R%d",reg_counter++);
                reg1 = tmp;
                if(is_number(r1)) append(asm_code,"LDF   %s, #%s\n",reg1,r1);
                else append(asm_code,"LDF   %s, %s\n",reg1,r1);
            }

            // operand 2
            if(r2[0]=='t') reg2 = get_reg(r2,&reg_counter);
            else {
                char tmp[4]; sprintf(tmp,"R%d",reg_counter++);
                reg2 = tmp;
                if(is_number(r2)) append(asm_code,"LDF   %s, #%s\n",reg2,r2);
                else append(asm_code,"LDF   %s, %s\n",reg2,r2);
            }

            // output
            char outreg[4]; sprintf(outreg,"R%d",reg_counter++);
            regout = outreg;
            const char *map_reg = get_reg(lhs,&reg_counter);
            if(!map_reg) {
                strncpy(temp_map[temp_map_count].temp,lhs,sizeof(temp_map[temp_map_count].temp)-1);
                strncpy(temp_map[temp_map_count].reg,outreg,sizeof(temp_map[temp_map_count].reg)-1);
                temp_map_count++;
            }

            // operation
            if(strcmp(op,"*")==0) append(asm_code,"MULF  %s, %s, %s\n",outreg,reg1,reg2);
            else if(strcmp(op,"/")==0) append(asm_code,"DIVF  %s, %s, %s\n",outreg,reg1,reg2);
            else if(strcmp(op,"+")==0) append(asm_code,"ADDF  %s, %s, %s\n",outreg,reg1,reg2);
            else if(strcmp(op,"-")==0) append(asm_code,"SUBF  %s, %s, %s\n",outreg,reg1,reg2);
        }
        // ---------- assignment ----------
        else if(sscanf(line,"%63s = %63s",lhs,r1)==2){
            const char *reg1;
            if(r1[0]=='t') reg1 = get_reg(r1,&reg_counter);
            else {
                char tmp[4]; sprintf(tmp,"R%d",reg_counter++);
                reg1 = tmp;
                if(is_number(r1)) append(asm_code,"LDF   %s, #%s\n",reg1,r1);
                else append(asm_code,"LDF   %s, %s\n",reg1,r1);
            }

            append(asm_code,"STF   %s, %s\n",lhs,reg1);
        }
    }
}

void build_symtext(char* out, int max) {
    // simple version: list all symbols
    snprintf(out, max, "ID    Variable   Type\n");
    for(int i=0;i<symcount;i++){
        char buf[128];
        snprintf(buf, sizeof(buf), "id%d    %-8s   %s\n", i+1, symtab[i].name, symtab[i].type);
        strncat(out, buf, max - strlen(out) - 1);
    }
}

// ----------------- Main -----------------
int main(){
   
    char expr[1024]; expr[0]=0;
    if (!fgets(expr, sizeof(expr), stdin)) {
        fprintf(stderr, "No input\n");
        return 1;
    }
    size_t L = strlen(expr);
    while (L>0 && (expr[L-1]=='\n' || expr[L-1]=='\r')) { expr[L-1]=0; L--; }

    tokcount = 0;
    symcount = 0;
    tokens_out[0]=normalized[0]=parse_tree[0]=varmap[0]=semantic_tree[0]=var_types[0]=tac[0]=opt_tac[0]=opt_info[0]=asm_code[0]=0;
    tmp_counter = 1;
    had_error = 0; error_phase[0]=0; error_msg[0]=0;

    scan_lexical(expr);

    Node* ast = NULL;
    if (!had_error) {
        curtok = 0;
        ast = parse_assignment();
        if (curToken()->type != TOK_END) {
            set_error("syntax", "Unexpected token '%s' after complete expression", curToken()->lexeme);
        }
    }

    parse_tree[0]=0;
    if (ast) print_parse_ascii(ast, "", 0, parse_tree);
    else append(parse_tree, "(no parse tree due to errors)\n");

    char varmap_local[512] = {0};
    append(varmap_local, "ID    Variable\n");
    for (int i=0;i<symcount;i++) append(varmap_local, "id%d    %s\n", i+1, symtab[i].name);
    strncpy(varmap, varmap_local, sizeof(varmap)-1);

    semantic_tree[0]=0; var_types[0]=0;
    if (!had_error && ast) {
        mark_ids_float_recursive(ast);
        append(semantic_tree, "Semantic Tree:\n");
        print_semantic_ascii(ast, "", 0, semantic_tree);
        append(semantic_tree, "\n");
        append(var_types, "ID    Variable   Type\n");
        for (int i=0;i<symcount;i++) append(var_types, "id%d    %-8s   %s\n", i+1, symtab[i].name, symtab[i].type);
    } else {
        append(semantic_tree, "(semantic analysis skipped due to earlier error)\n");
        append(var_types, "ID    Variable   Type\n");
        for (int i=0;i<symcount;i++) append(var_types, "id%d    %-8s   %s\n", i+1, symtab[i].name, symtab[i].type);
    }

    tac[0]=opt_tac[0]=opt_info[0]=asm_code[0]=0;
    if (!had_error && ast) {
        tmp_counter = 1;
        gen_tac_rec(ast, (char[]){0});
        optimize_tac();
        generate_assembly_optimized();  // <-- ekhane change
    } else {
        append(tac, "(skipped due to earlier error)\n");
        append(opt_tac, "(skipped due to earlier error)\n");
        append(opt_info, "(skipped due to earlier error)\n");
        append(asm_code, "(skipped due to earlier error)\n");
    }

    char symtxt[512]; build_symtext(symtxt, sizeof(symtxt));

    char tokens_json[4096], symtxt_json[4096], normalized_json[1024];
    char parse_tree_json[4096], varmap_json[1024], semantic_tree_json[8192];
    char var_types_json[1024], tac_json[4096], opt_tac_json[4096];
    char opt_info_json[1024], asm_json[4096], error_phase_json[128], error_msg_json[1024];

    json_escape(tokens_json, tokens_out, sizeof(tokens_json));
    json_escape(symtxt_json, symtxt, sizeof(symtxt_json));
    json_escape(normalized_json, normalized, sizeof(normalized_json));
    json_escape(parse_tree_json, parse_tree, sizeof(parse_tree_json));
    json_escape(varmap_json, varmap, sizeof(varmap_json));
    json_escape(semantic_tree_json, semantic_tree, sizeof(semantic_tree_json));
    json_escape(var_types_json, var_types, sizeof(var_types_json));
    json_escape(tac_json, tac, sizeof(tac_json));
    json_escape(opt_tac_json, opt_tac, sizeof(opt_tac_json));
    json_escape(opt_info_json, opt_info, sizeof(opt_info_json));
    json_escape(asm_json, asm_code, sizeof(asm_json));
    json_escape(error_phase_json, error_phase, sizeof(error_phase_json));
    json_escape(error_msg_json, error_msg, sizeof(error_msg_json));

    printf("{\n");
    printf("\"lexical\": { \"tokens\": \"%s\", \"symtab\":\"%s\", \"normalized\":\"%s\" },\n",
        tokens_json, symtxt_json, normalized_json);
    printf("\"syntax\": { \"parse_tree\": \"%s\", \"varmap\":\"%s\" },\n",
        parse_tree_json, varmap_json);
    printf("\"semantic\": { \"sem_tree\": \"%s\", \"var_types\":\"%s\" },\n",
        semantic_tree_json, var_types_json);
    printf("\"intermediate\": { \"tac\": \"%s\" },\n", tac_json);
    printf("\"optimization\": { \"opt_tac\": \"%s\", \"info\":\"%s\" },\n",
        opt_tac_json, opt_info_json);
    printf("\"assembly\": { \"asm\": \"%s\" },\n", asm_json);

    if (had_error) {
        printf("\"error\": { \"phase\": \"%s\", \"message\": \"%s\" }\n", error_phase_json, error_msg_json);
    } else {
        printf("\"error\": null\n");
    }
    printf("}\n");

    return 0;
}
