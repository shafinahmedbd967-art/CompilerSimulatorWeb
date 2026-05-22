🖥️ CompilerSimulatorWeb
Stepwise Learning Tool for Compiler Phases

A web-based interactive compiler simulation platform that visually demonstrates all major phases of compilation for arithmetic expressions — built as a Mini Lab Project for CSE314: Compiler Design Lab at Daffodil International University.


📌 Overview
CompilerSimulatorWeb transforms abstract compiler theory into a hands-on, visual learning experience. Users input an arithmetic expression in the browser and watch it get processed through six compiler phases in real time — complete with token tables, parse trees, symbol tables, TAC, optimized code, and assembly output.

⚙️ Compilation Phases Simulated
PhaseDescription🔍 Lexical AnalysisTokenizes input, builds symbol table, detects lexical errors🌳 Syntax AnalysisBuilds parse tree, validates grammar & operator precedence🧠 Semantic AnalysisType checking, float propagation, semantic tree generation📄 Intermediate Code (TAC)Generates Three-Address Code using temp variables⚡ Code OptimizationConstant folding, dead code elimination🔩 Assembly Code GenerationProduces NASM-style assembly instructions

🛠️ Tech Stack
Frontend: HTML5, CSS3, Vanilla JavaScript (Fetch API)
Backend: Node.js + Express.js
Compiler Core: C (compiler.c)
Optional: Flex (lexical.l), Bison (syntax.y)

📁 Project Structure
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

🚀 Getting Started
Prerequisites

Node.js installed
GCC compiler (for building compiler.c)

1. Clone the repository
bashgit clone https://github.com/YOUR_USERNAME/CompilerSimulatorWeb.git
cd CompilerSimulatorWeb
2. Build the C compiler
bashcd backend
gcc compiler.c -o compiler.exe -lm
3. Install Node.js dependencies
bashcd ../node_server
npm install
4. Start the server
bashnode server.js
5. Open in browser
http://localhost:3000

🧪 Example Expressions
x = a * 3 - b / c + 9 - e * 5.5
y = 2 + 3 * z
x = a * (3 - b) / c
x = a ** 3   ← triggers error detection

📊 Performance
ExpressionTokensTotal Response Timex = a + b * 35~3 msx = a * (3 - b) / c + 99 - 55.0 * g13~10 msx = (a + b * c) / (d - e * f) + 4215~12 ms

👥 Team
NameStudent IDRoleShafin Ahmed232-15-184Project Lead / Lead DeveloperMD Foysal Bhuiyan232-15-897Compiler Logic DeveloperSafayet Abir232-15-225Frontend DeveloperAbu Dazana232-15-810Documentation & Semantic ModuleBijoy Krishna Sarker232-15-219Testing & QA

Note: Core implementation (compiler.c, system architecture, backend-frontend integration) developed by Shafin Ahmed.


📚 Course Info
Course: CSE314 – Compiler Design Lab
Section: 65-H2, Group 05
Supervisor: Rabeya Khatun, Lecturer, CSE Dept.
Institution: Daffodil International University, Dhaka, Bangladesh
Submitted: December 14, 2025

📖 References

Aho, Sethi & Ullman — Compilers: Principles, Techniques, and Tools (Dragon Book)
Levine — flex & bison
Node.js Docs
MDN JavaScript


📄 License
This project is for academic/educational purposes. Feel free to fork and extend it.
