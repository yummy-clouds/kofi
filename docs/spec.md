# Kofi Language Specification

## 1. Overview
Kofi is a statically typed compiled programming language designed for simplicity and performance. It compiles source code (`.kf`) to C and subsequently produces native machine binaries via GCC.

## 2. Data Types
Kofi features five core data types:

| Type | Description | Example Literal | C Mapping |
| :--- | :--- | :--- | :--- |
| `int` | Integer numbers | `42`, `-10`, `0` | `int` / `long long` |
| `flt` | Floating-point numbers | `3.14`, `0.5`, `-2.0` | `double` |
| `bln` | Boolean truth values | `true`, `false` | `bool` |
| `stg` | UTF-8 String literals | `"hello, world"` | `const char*` |
| `nil` | Uninitialized value / nil | `nil` | `NULL` (`void*`) |

### Nil and Uninitialized Values
`nil` represents the absence of a value or an uninitialized state.
- A variable declared without an explicit initial value evaluates to its type's default value or `nil`:
  - `stg` and `nil` default to `nil`.
  - `var x` (with no type or value) defaults to type `nil` with value `nil`.
  - Printing `nil` or an uninitialized string outputs `nil`.

## 3. Variables
Variables are declared using the `var` keyword.

### Syntax
```kf
var <name>: <type> = <expr>   // explicit type & initialization
var <name>: <type>            // explicit type, default/nil value
var <name> = <expr>           // type inferred from expression
var <name>                    // uninitialized (type nil, value nil)
```

### Assignment
Existing variables can be reassigned with `=`:
```kf
<name> = <expr>
```

## 4. Control Flow

### `for` Loop
Kofi uses a single looping keyword, `for`. It supports three distinct forms:

#### 1. Three-Clause Loop
```kf
for var i: int = 0; i < 10; i = i + 1 {
    printf("", i, "\n")
}
```

#### 2. While-Style Condition Loop
```kf
var count: int = 5
for count > 0 {
    printf("", count, "\n")
    count = count - 1
}
```

#### 3. Infinite Loop
```kf
for {
    // runs indefinitely until break
    if condition {
        break
    }
}
```

#### Loop Control Statements
- `break`: Terminates the innermost loop immediately.
- `continue`: Advances to the next iteration of the innermost loop.

### Conditional Statement (`if` / `else`)
```kf
if x > 0 {
    printf("positive\n")
} else if x == 0 {
    printf("zero\n")
} else {
    printf("negative\n")
}
```

## 5. Functions
Functions are declared with the `fc` keyword:
```kf
fc <name>(<param1>: <type>, <param2>: <type>) => <return_type> {
    return <expr>
}
```
If a function does not return a value, the `=> <return_type>` clause may be omitted.

The entry point of every Kofi program is the `main` function:
```kf
fc main() {
    printf("hello from kofi\n")
}
```

## 6. Built-in I/O Functions

### Output: `printf`
Kofi uses `printf` as its single output function. No automatic newlines are appended; newlines are added using `\n`.
Traditional formatting specifiers like `%d` are not required—you can use `printf("", a)` or pass labels and arguments directly:
- `printf("hello")`: Prints string without newline.
- `printf("hello\n")`: Prints string with newline.
- `printf("", a)`: Prints the value of `a` directly.
- `printf("", a, "\n")`: Prints the value of `a` followed by a newline.
- `printf("Value is: ", a, "\n")`: Prints label, value, and newline.

### Input: `scanf`
Kofi uses `scanf` as its single input function. Pointers (`&a`) are NOT needed; the variable is passed directly:
- `scanf("enter a num: ", a)`: Displays the prompt and reads the value into `a`.
- `scanf("", a)` or `scanf(a)`: Reads the value into `a` without a prompt.
- Supports all core types (`int`, `flt`, `bln`, `stg`).
