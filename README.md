# Kappok Language Interpreter

Kappok is a lightweight and beginner-friendly programming language designed for education, experimentation, and simplicity. It supports fundamental data types, variable declarations, arithmetic operations, built-in functions, and user-defined functions with a concise syntax.

---

## Key Features

### Supported Data Types

- int: Signed integer  
- double: Double-precision floating-point number  
- str: String  
- bool: Boolean (True, False)  
- void: Represents no value (mainly used for main function return type or special cases)  
- function: User-defined function  

### Variable Declaration and Assignment

Example:

    int x = 10
    double pi = 3.14
    str name = "Kappok"
    bool isActive = True

Reassignment:

    x = 20
    name = "NewKappok"

### Arithmetic Operators

Supported operators:

- + : Addition  
- - : Subtraction  
- * : Multiplication  
- / : Division  

Type conversion rules:

- int and int -> int  
- if one operand is double, the other (int or bool) is converted to double, and the result is double

### Built-in Functions

#### print(...)

Prints one or more expressions to standard output, separated by spaces, ending with a newline.

Example:

    print("Hello", "World!")
    print(10 + 5)

When printing double values, trailing zeros are removed.  
Examples:

    print(3.0)       # outputs 3
    print(3.1400)    # outputs 3.14

#### round(value, precision)

Rounds a number to a specified number of decimal places and returns it as a string.

Format:

    round(number, digits)

- number: int or double  
- digits: number of decimal places (int)

Examples:

    round(3.14159, 2)    # returns "3.14"
    round(6.28318, 4)    # returns "6.2832"

Trailing zeros are preserved to match the precision.

### User-defined Functions

Functions are defined using def. The main() function is the program entry point.

Example:

    def main() {
        int r = 2
        double pi = 3.14159
        print("Area is", round(r*r*pi/2, 4))
        return r * r * pi / 2
    }

---

## Execution (Planned)

- Programs start from the main() function.  
- The interpreter will parse and execute .kpp files.  
- Lexical analysis, parsing, and evaluation are under development.

---

## License

This project is distributed under the Apache License 2.0.  
https://www.apache.org/licenses/LICENSE-2.0
