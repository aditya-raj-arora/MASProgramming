# MAS Programming Language

**MAS** is a minimal, interpreted programming language designed for learning compiler and interpreter implementation. It features a clean syntax inspired by Python, with support for variables, arithmetic, loops, conditionals, lists, and built-in functions like `print`.

> **MAS** stands for **Minimal And Simple** — a language built from scratch to understand the core concepts of programming language design.

---

## 🌟 Features

- **Dynamic typing** with automatic type inference
- **First-class lists** with literal syntax: `[1, 2, 3]`
- **Control structures**: `if`, `loop`, `each` (for-each loop)
- **Functions**: Built-in `print`, with user-defined functions (WIP)
- **Expressions**: Arithmetic (`+`, `-`, `*`, `/`), comparisons (`==`, `!=`, `<`, etc.)
- **Clean syntax**: No semicolons, indentation-like structure using `end`
- **Memory management**: Reference counting for automatic cleanup

---

## 🚀 Example Program

```python
# Hello World
print "Hello, MAS!"

# Variables and math
x = 10
y = x * 2 + 5
print "Result:", y

# Lists
fruits = ["apple", "banana", "cherry"]
print fruits

# Loop
i = 0
loop i < 3:
    print "Count:", i
    i = i + 1
end

# For-each loop
each f in fruits:
    print "Fruit:", f
end

each i in 1 to 3:
    print "Count:", i
end

def add(a,b):
    give a+b
end

print add(2,3)
```

**Output:**
```
Hello, MAS!
Result: 25
[apple, banana, cherry]
Count: 0
Count: 1
Count: 2
Fruit: apple
Fruit: banana
Fruit: cherry
Count: 1
Count: 2
Count: 3
5
```

---

## 🛠️ Building & Running

### Prerequisites
- A C compiler (GCC or Clang)
- Make (optional)

### Build
```bash
git clone https://github.com/aditya-raj-arora/MASProgramming.git
cd MASProgramming/mas
gcc -o mas main.c lexer.c parser.c interpreter.c -lm
```

### Run a Program
```bash
./mas your_program.mas
```

> Replace `your_program.mas` with the path to your MAS source file.

---

## 📁 Project Structure

```
mas/
├── mas.h           # Shared headers and type definitions
├── lexer.c         # Tokenizer (converts source code to tokens)
├── parser.c        # Recursive descent parser (builds AST)
├── interpreter.c   # Tree-walking interpreter (executes AST)
├── main.c          # Entry point and driver
└── test.mas        # Example MAS program
```

---

## 🔧 Language Syntax

### Variables
```mas
x = 42
name = "MAS"
```

### Control Flow
```mas
# Conditional
if x > 10:
    print "Big number"
end

# Loop
i = 0
loop i < 5:
    print i
    i = i + 1
end

# For-each
items = [10, 20, 30]
each item in items:
    print item
end
```

### Data Types
- **Number**: `42`, `3.14`
- **String**: `"hello"` (supports `\n`, `\t`, `\"`)
- **Boolean**: `true`, `false`
- **Null**: `null`
- **List**: `[1, "two", true]`

### Operators
- Arithmetic: `+`, `-`, `*`, `/`
- Comparison: `==`, `!=`, `<`, `<=`, `>`, `>=`
- Assignment: `=`

---

## 🧪 Testing

A sample test file is included:

```bash
./mas test.mas
```

This runs basic examples of variables, expressions, loops, and the `print` function.

---

## 📚 Learning Goals

This project demonstrates core concepts in programming language implementation:
- Lexical analysis (tokenization)
- Recursive descent parsing
- Abstract Syntax Trees (AST)
- Tree-walking interpreters
- Runtime object model and memory management

It's ideal for students, hobbyists, or anyone wanting to build their first language!

---

## 🤝 Contributing

Contributions are welcome! Feel free to:
- Fix bugs
- Add new features (e.g., user-defined functions, more data types)
- Improve error messages
- Write more test cases

Just open a pull request with your changes.

---

## 📜 License

This project is open-source and available under the **MIT License**. See [LICENSE](LICENSE) for details.

---

> **Built with ❤️ for learners and creators.**  
> — Aditya Raj Arora
> — Mukta Motwani
> — Shivam Saini
