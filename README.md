# 🖥️ CompilerSimulatorWeb
### Stepwise Learning Tool for Compiler Phases

> A web-based interactive compiler simulation platform that visually demonstrates all major phases of compilation for arithmetic expressions — built as a Mini Lab Project for **CSE314: Compiler Design Lab** at Daffodil International University.

---

## 📌 Overview

**CompilerSimulatorWeb** transforms abstract compiler theory into a hands-on, visual learning experience. Users input an arithmetic expression in the browser and watch it get processed through six compiler phases in real time — complete with token tables, parse trees, symbol tables, TAC, optimized code, and assembly output.

---

## ⚙️ Compilation Phases Simulated

| Phase | Description |
|---|---|
| 🔍 Lexical Analysis | Tokenizes input, builds symbol table, detects lexical errors |
| 🌳 Syntax Analysis | Builds parse tree, validates grammar & operator precedence |
| 🧠 Semantic Analysis | Type checking, float propagation, semantic tree generation |
| 📄 Intermediate Code (TAC) | Generates Three-Address Code using temp variables |
| ⚡ Code Optimization | Constant folding, dead code elimination |
| 🔩 Assembly Code Generation | Produces NASM-style assembly instructions |

---

## 🛠️ Tech Stack

**Frontend:** HTML5, CSS3, Vanilla JavaScript (Fetch API)  
**Backend:** Node.js + Express.js  
**Compiler Core:** C (`compiler.c`)  
**Optional:** Flex (`lexical.l`), Bison (`syntax.y`)

---

## 📁 Project Structure

```
CompilerSimulatorWeb/
├── backend/
│   ├── compiler.c        # Full compiler pipeline in C
│   ├── compiler.exe      # Compiled binary
│   ├── lexical.l         # Flex lexer (optional)
│   ├── syntax.y          # Bison parser (optional)
│   └── Makefile
├── frontend/
│   ├── index.html        # Main UI
│   ├── script.js         # Fetch API + output rendering
│   └── style.css         # Dark terminal theme
├── node_server/
│   ├── server.js         # Express server, spawns compiler
│   ├── package.json
│   └── package-lock.json
└── temp/
    └── input.txt         # Temporary expression file
```

---

## 🚀 Getting Started

### Prerequisites
- [Node.js](https://nodejs.org/) installed
- GCC compiler (for building `compiler.c`)

### 1. Clone the repository
```bash
git clone https://github.com/shafinahmedbd967-art/CompilerSimulatorWeb.git
cd CompilerSimulatorWeb
```

### 2. Build the C compiler
```bash
cd backend
gcc compiler.c -o compiler.exe -lm
```

### 3. Install Node.js dependencies
```bash
cd ../node_server
npm install
```

### 4. Start the server
```bash
node server.js
```

### 5. Open in browser
```
http://localhost:3000
```

---

## 🧪 Example Expressions

```
x = a * 3 - b / c + 9 - e * 5.5
y = 2 + 3 * z
x = a * (3 - b) / c
x = a ** 3   ← triggers error detection
```

---

## 📊 Performance

| Expression | Tokens | Total Response Time |
|---|---|---|
| `x = a + b * 3` | 5 | ~3 ms |
| `x = a * (3 - b) / c + 99 - 55.0 * g` | 13 | ~10 ms |
| `x = (a + b * c) / (d - e * f) + 42` | 15 | ~12 ms |

---

## 📚 Course Info

**Course:** CSE314 – Compiler Design Lab  
**Section:** 65-H2 
**Supervisor:** Rabeya Khatun, Lecturer, CSE Dept.  
**Institution:** Daffodil International University, Dhaka, Bangladesh  
**Submitted:** December 14, 2025

---

## 📖 References

- Aho, Sethi & Ullman — *Compilers: Principles, Techniques, and Tools* (Dragon Book)
- Levine — *flex & bison*
- [Node.js Docs](https://nodejs.org/en/docs/)
- [MDN JavaScript](https://developer.mozilla.org/en-US/docs/Web/JavaScript)

---

## 📄 License

This project is for academic/educational purposes.  
Developed by **[Shafin Ahmed](https://github.com/shafinahmedbd967-art)**
