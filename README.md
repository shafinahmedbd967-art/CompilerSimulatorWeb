# 🖥️ CompileX — Interactive Compiler Simulator

<div align="center">

### 🔬 Visualizing Compiler Design, One Phase at a Time

An interactive web-based compiler simulation platform that demonstrates the complete compilation process for arithmetic expressions through visual representations, intermediate code generation, optimization, and assembly code generation.

Built as a Mini Lab Project for **CSE314: Compiler Design Lab** at **Daffodil International University**.

![Compiler](https://img.shields.io/badge/Compiler-Simulator-blue)
![C](https://img.shields.io/badge/C-Language-blue)
![Node.js](https://img.shields.io/badge/Node.js-Backend-green)
![Express](https://img.shields.io/badge/Express.js-Framework-black)
![Status](https://img.shields.io/badge/Status-Completed-success)

</div>

---

# 📖 Project Overview

Understanding compiler design can be challenging because most compilation processes happen behind the scenes.

**CompileX** bridges this gap by providing a visual, interactive environment where users can enter arithmetic expressions and observe how a compiler processes them step by step.

The platform simulates major compiler phases including:

* Lexical Analysis
* Syntax Analysis
* Semantic Analysis
* Intermediate Code Generation
* Code Optimization
* Assembly Code Generation

This enables students to gain a practical understanding of compiler internals rather than relying solely on theoretical concepts.

---

# 🎯 Objectives

The primary goals of this project are:

* Demonstrate compiler phases visually
* Simplify compiler design concepts
* Help students understand parsing and code generation
* Generate intermediate representations
* Show optimization techniques in action
* Simulate real-world compiler behavior

---

# ⚙️ Compiler Pipeline

The system processes an input expression through six major stages.

---

## 🔍 Phase 1: Lexical Analysis

The lexical analyzer scans the source code and converts it into tokens.

### Responsibilities

* Token generation
* Keyword detection
* Identifier recognition
* Constant detection
* Symbol table creation
* Lexical error detection

### Example

Input:

```text
x = a * 3 + b
```

Generated Tokens:

```text
IDENTIFIER : x
ASSIGN     : =
IDENTIFIER : a
OPERATOR   : *
NUMBER     : 3
OPERATOR   : +
IDENTIFIER : b
```

---

## 🌳 Phase 2: Syntax Analysis

Builds a parse tree based on grammar rules and validates expression structure.

### Features

* Operator precedence handling
* Parentheses validation
* Grammar checking
* Parse tree generation

Example:

```text
      +
     / \
    *   b
   / \
  a   3
```

---

## 🧠 Phase 3: Semantic Analysis

Verifies semantic correctness.

### Checks

* Type compatibility
* Variable usage
* Float propagation
* Semantic validation

Outputs:

* Semantic Tree
* Type Information
* Semantic Errors

---

## 📄 Phase 4: Intermediate Code Generation

Generates Three Address Code (TAC).

### Example

Expression:

```text
x = a * 3 + b
```

Generated TAC:

```text
t1 = a * 3
t2 = t1 + b
x = t2
```

---

## ⚡ Phase 5: Code Optimization

Optimizes intermediate code to improve efficiency.

### Techniques Used

* Constant Folding
* Redundant Instruction Removal
* Dead Code Elimination
* Temporary Variable Reduction

Example:

Before:

```text
t1 = 5 + 5
x = t1
```

After:

```text
x = 10
```

---

## 🔩 Phase 6: Assembly Code Generation

Converts optimized TAC into NASM-style assembly instructions.

Example:

```asm
MOV AX, a
MUL 3
ADD AX, b
MOV x, AX
```

---

# 🌟 Key Features

### Interactive Compiler Visualization

* Real-time processing
* Phase-by-phase execution
* Educational interface

### Symbol Table Generation

Displays:

* Variable names
* Data types
* Memory references

### Parse Tree Visualization

Shows hierarchical expression structure.

### TAC Generator

Automatically creates Three Address Code.

### Optimization Engine

Demonstrates compiler optimization techniques.

### Assembly Generator

Produces assembly-like output.

### Error Detection

Handles:

* Invalid symbols
* Syntax errors
* Semantic errors

---

# 🛠 Technology Stack

## Frontend

* HTML5
* CSS3
* Vanilla JavaScript
* Fetch API

## Backend

* Node.js
* Express.js

## Compiler Core

* C Programming Language

## Optional Compiler Tools

* Flex
* Bison

---

# 📁 Project Structure

```text
CompilerSimulatorWeb/
│
├── backend/
│   ├── compiler.c
│   ├── compiler.exe
│   ├── lexical.l
│   ├── syntax.y
│   └── Makefile
│
├── frontend/
│   ├── index.html
│   ├── script.js
│   └── style.css
│
├── node_server/
│   ├── server.js
│   ├── package.json
│   └── package-lock.json
│
└── temp/
    └── input.txt
```

---

# 🚀 Getting Started

## Prerequisites

Install:

* Node.js (v18+ recommended)
* GCC Compiler

Verify installation:

```bash
node -v
gcc --version
```

---

## Clone Repository

```bash
git clone https://github.com/shafinahmedbd967-art/CompilerSimulatorWeb.git
cd CompilerSimulatorWeb
```

---

## Build Compiler

```bash
cd backend
gcc compiler.c -o compiler.exe -lm
```

---

## Install Dependencies

```bash
cd ../node_server
npm install
```

---

## Start Server

```bash
node server.js
```

---

## Open Browser

```text
http://localhost:3000
```

---

# 🧪 Sample Inputs

```text
x = a * 3 - b / c + 9 - e * 5.5
```

```text
y = 2 + 3 * z
```

```text
x = a * (3 - b) / c
```

```text
x = a ** 3
```

Expected:

```text
Lexical Error:
Unsupported operator '**'
```

---

# 📊 Performance Snapshot

| Expression Complexity | Tokens | Avg Response Time |
| --------------------- | ------ | ----------------- |
| Simple                | 5      | ~3 ms             |
| Medium                | 13     | ~10 ms            |
| Complex               | 15+    | ~12 ms            |

---

# 🎓 Academic Information

### Course

**CSE314 – Compiler Design Lab**

### Institution

**Daffodil International University**

### Department

Department of Computer Science & Engineering

### Supervisor

**Rabeya Khatun**
Lecturer, Department of CSE

### Submission Date

December 14, 2025

---

# 📚 Learning Outcomes

After completing this project, students can understand:

* Lexical Analysis
* Parsing Techniques
* Parse Trees
* Symbol Tables
* Semantic Analysis
* Intermediate Code Generation
* Compiler Optimization
* Assembly Code Generation

---

# 🔮 Future Improvements

Potential enhancements include:

* Full language grammar support
* AST visualization
* LR Parser simulation
* Register allocation
* Live syntax highlighting
* WebAssembly integration
* Control flow graph generation
* SSA representation

---

# 📖 References

### Books

* Aho, Sethi & Ullman — *Compilers: Principles, Techniques and Tools*
* Levine — *flex & bison*

### Documentation

* Node.js Documentation
* Express.js Documentation
* GCC Documentation
* MDN Web Docs

---

# 👨‍💻 Author

## Shafin Ahmed

GitHub:

https://github.com/shafinahmedbd967-art

---

# 📜 License

This project is developed for educational and academic purposes.

---

<div align="center">

### 🚀 Making Compiler Design Interactive & Understandable

Built with ❤️ using C, Node.js, Express, and JavaScript

</div>
