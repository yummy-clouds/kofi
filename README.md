# Kofi Programming Language

Kofi is a lightweight, compiled programming language that transpiles to C and compiles directly to native binaries.

## Features
- **5 Core Data Types**:
  - `int`: Integers (e.g. `42`, `-7`)
  - `flt`: Floating-point numbers (e.g. `3.14`, `0.5`)
  - `bln`: Booleans (`true`, `false`)
  - `stg`: Strings (e.g. `"hello world"`)
  - `nil`: Uninitialized value / nil (`nil`)
- **Variables (`var`)**:
  - Explicit typed declarations: `var x: int = 10`
  - Inferred declarations: `var y = 3.14`
  - Uninitialized declarations: `var z: stg` (defaults to `nil`)
- **Single Go-Style Looping Statement (`for`)**:
  - Three-clause loop: `for var i: int = 0; i < 10; i = i + 1 { ... }`
  - While-style condition loop: `for count > 0 { ... }`
  - Infinite loop: `for { ... break }`
  - Loop control: `break` and `continue`
- **Control Flow**:
  - `if`, `else if`, `else`
- **Output: `printf`**:
  - Single output statement without forced newlines (use `\n`).
  - No format specifiers needed: `printf("", a)` or `printf("Value: ", a, "\n")`.
- **Input: `scanf`**:
  - Single input statement with optional prompt and without `&` pointers: `scanf("enter a num: ", a)`.

## Building the Compiler

With `gcc` installed:
```bash
mingw32-make
# or on Linux / macOS:
make
```

Or manually:
```bash
gcc -Wall -Wextra -std=c99 -g src/main.c src/lexer.c src/parser.c src/codegen.c src/util.c -o kofi.exe
```

## Running Kofi Programs

Compile a `.kf` file:
```bash
./kofi.exe examples/io.kf
# then run the generated output binary:
./output.exe
```

## Running Tests

On Windows PowerShell:
```powershell
powershell -ExecutionPolicy Bypass -File tests/run_tests.ps1
```

On Linux / macOS / Bash:
```bash
bash tests/run_tests.sh
```