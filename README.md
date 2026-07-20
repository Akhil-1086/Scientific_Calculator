<div align="center">

# Scientific Calculator using Arduino Uno

### AVR-Based Scientific Calculator with Custom Mathematical Function Implementation


</div>

---

# Overview

This project implements a **scientific calculator** on the **ATmega328P (Arduino Uno)** using a **16×2 LCD display** and a matrix of push buttons. Instead of relying entirely on standard library routines, several mathematical functions are implemented using classical numerical methods.

The calculator supports both basic arithmetic operations and scientific computations while maintaining compatibility with resource-constrained embedded systems.

---

# Features

### Arithmetic Operations

- Addition
- Subtraction
- Multiplication
- Division

### Scientific Functions

- Sine
- Cosine
- Square Root
- Exponential
- Natural Logarithm
- Common Logarithm
- Inverse Sine
- Inverse Cosine
- Inverse Tangent

### Software Features

- Recursive Descent Expression Parser
- LCD Driver
- Button Scanning Interface
- Custom Mathematical Library
- AVR-GCC Compatible

---

# Hardware Requirements

| Component | Quantity |
|------------|---------:|
| Arduino Uno | 1 |
| 16×2 LCD Display | 1 |
| Push Buttons | 12 |
| Potentiometer | 1 |
| Breadboard | 1 |
| Jumper Wires | As required |

---

# Hardware Connections

| Component | Arduino Pin |
|-----------|-------------|
| LCD RS | D12 |
| LCD EN | D11 |
| LCD Data (D4–D7) | D5, D4, D3, D2 |
| Numeric Buttons | D6–D10, A0–A4 |
| Mode Selection Buttons | A5, D13 |
| Potentiometer | LCD Contrast (V0) |

---

# Software Architecture

```
                    User Input
                         │
                         ▼
                 Button Detection
                         │
                         ▼
               Expression Formation
                         │
                         ▼
          Recursive Descent Parser
                         │
          ┌──────────────┴──────────────┐
          ▼                             ▼
 Arithmetic Evaluation         Scientific Functions
          ▼                             ▼
             Mathematical Engine
                         │
                         ▼
                    LCD Display
```

---

# Mathematical Algorithms

| Function | Method |
|----------|--------|
| sin(x) | Taylor Series |
| cos(x) | Taylor Series |
| √x | Newton-Raphson Method |
| eˣ | Euler Approximation |
| Expression Evaluation | Recursive Descent Parsing |

---

# Operating Modes

## Normal Mode

- Numeric Input
- Basic Arithmetic Operations
- Backspace
- Evaluation

## Shift Mode

- Sine
- Cosine
- Square Root
- Exponential

## Extra Mode

- Inverse Trigonometric Functions
- Natural Logarithm
- Common Logarithm

---

# Directory Structure

```text
Scientific-Calculator
│
├── main.c
├── lcd.c
├── lcd.h
├── Makefile
│
├── images
│   ├── hardware.jpg
│   ├── lcd_display.png
│
├── report
│   └── CALCULATOR.pdf
│
└── README.md
```

---



---



# Future Enhancements

- Parentheses support
- Memory operations
- Improved numerical accuracy
- Degree/Radian switching
- Matrix operations
- Additional scientific functions


