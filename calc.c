/*
 * Embedded C Calculator for AVR (using avr-gcc)
 * Fixed version: compile error corrected, scientific functions wired into
 * the parser, buffer bounds checked, angle reduction added, unary minus added.
 *
 * NOTE: You must supply an LCD driver (lcd.h) that provides lcd_init(),
 * lcd_clear(), and lcd_print(char*) functions. Also, adapt the button
 * reading functions to your hardware.
 */

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "lcd.h"

#ifndef PI
  #define PI 3.14159265358979323846
#endif

// -------------------------
// Hardware Definitions
// -------------------------
#define BUTTON_PORT   PIND
#define SHIFT_PORT    PINB
#define EXTRA_PORT    PINC

#define BUTTON_MASK(i)    (1 << (i))
#define SHIFT_BUTTON_MASK (1 << 0)
#define EXTRA_BUTTON_MASK (1 << 0)

// -------------------------
// Calculator Global Variables
// -------------------------
#define INPUT_SIZE 128
char input[INPUT_SIZE] = "";

bool shiftActive = false;
bool extraActive = false;

bool lastShiftState = true;
bool lastExtraState = true;

// -------------------------
// Mapping for Keys
// -------------------------
char normalMode[10] = {'0','1','2','3','4','5','6','7','8','9'};
char shiftOps[6]  = {'+', '-', '*', '/', '=', '<'};
const char* shiftFuncs[4] = {"sin", "cos", "e^", "sqrt"};
const char* extraFuncs[8] = {"asin", "acos", "atan", "log", "ln", "(", ")", "^"};

// -------------------------
// Function Prototypes
// -------------------------
void updateLCD(void);
void handleSpecial(char op);
float evaluateExpression(const char* expr);

float parseExpression(const char* s, int *pos);
float parseTerm(const char* s, int *pos);
float parseFactor(const char* s, int *pos);
float parseUnary(const char* s, int *pos);
float parsePrimary(const char* s, int *pos);
void skipSpaces(const char* s, int *pos);
bool appendSafe(const char* text);

float mySin(float x);
float myCos(float x);
float myExp(float x);
float mySqrt(float x);
float myLn(float x);
float myLog(float x);
float myAsin(float x);
float myAcos(float x);
float myAtan(float x);

// -------------------------
// Button Reading Functions (Placeholders)
// -------------------------
bool button_pressed(uint8_t pinMask) {
    return !(BUTTON_PORT & pinMask);
}

bool shift_button_pressed(void) {
    return !(SHIFT_PORT & SHIFT_BUTTON_MASK);
}

bool extra_button_pressed(void) {
    return !(EXTRA_PORT & EXTRA_BUTTON_MASK);
}

// -------------------------
// Math Functions Definitions
// -------------------------

// Reduce angle to (-180, 180] degrees so the Taylor series stays accurate
static float reduceDegrees(float deg) {
  deg = fmodf(deg, 360.0f);
  if (deg > 180.0f) deg -= 360.0f;
  if (deg < -180.0f) deg += 360.0f;
  return deg;
}

float mySin(float x){
  float rad = reduceDegrees(x) * PI / 180.0f;
  float term = rad;
  float sum = rad;
  for (int n = 1; n < 12; n++){
    term = -term * rad * rad / ((2 * n) * (2 * n + 1));
    sum += term;
  }
  return sum;
}

float myCos(float x){
  float rad = reduceDegrees(x) * PI / 180.0f;
  float term = 1;
  float sum = 1;
  for (int n = 1; n < 12; n++){
    term = -term * rad * rad / ((2 * n - 1) * (2 * n));
    sum += term;
  }
  return sum;
}

float myExp(float x){
  int N = 200;
  float h = x / N;
  float y = 1.0;
  for (int i = 0; i < N; i++){
    y = y * (1 + h);
  }
  return y;
}

float mySqrt(float x){
  if (x < 0) return -1;
  if (x == 0) return 0;
  float guess = x / 2.0f;
  if (guess == 0) guess = 1.0f;
  for (int i = 0; i < 20; i++){
    guess = (guess + x / guess) / 2.0f;
  }
  return guess;
}

float myLn(float x){
  if (x <= 0) return -1000000;
  float y = (x - 1) / (x + 1);
  float sum = 0;
  for (int n = 0; n < 20; n++){
    sum += powf(y, 2 * n + 1) / (2 * n + 1);
  }
  return 2 * sum;
}

float myLog(float x){
  return myLn(x) / myLn(10.0f);
}

// atan via series/argument reduction, valid for all real x
float myAtan(float x){
  bool neg = x < 0;
  if (neg) x = -x;
  bool invert = x > 1.0f;
  if (invert) x = 1.0f / x;

  float term = x;
  float sum = 0;
  float xx = x * x;
  for (int n = 0; n < 30; n++){
    sum += term / (2 * n + 1) * ((n % 2 == 0) ? 1 : -1);
    term *= xx;
  }
  float result = sum;
  if (invert) result = (PI / 2.0f) - result;
  if (neg) result = -result;
  return result * 180.0f / PI;
}

float myAsin(float x){
  if (x < -1 || x > 1) return 0;
  if (x == 1.0f) return 90.0f;
  if (x == -1.0f) return -90.0f;
  float denom = mySqrt(1 - x * x);
  return myAtan(x / denom);
}

float myAcos(float x){
  return 90.0f - myAsin(x);
}

// -------------------------
// Recursive Descent Parser Functions
// -------------------------
void skipSpaces(const char* s, int *pos) {
  while (s[*pos] == ' ' || s[*pos] == '\t')
    (*pos)++;
}

static bool matchWord(const char* s, int *pos, const char* word) {
  size_t len = strlen(word);
  if (strncmp(&s[*pos], word, len) == 0) {
    *pos += len;
    return true;
  }
  return false;
}

float parseExpression(const char* s, int *pos) {
  skipSpaces(s, pos);
  float value = parseTerm(s, pos);
  skipSpaces(s, pos);
  while (s[*pos] == '+' || s[*pos] == '-') {
    char op = s[(*pos)++];
    float term = parseTerm(s, pos);
    if (op == '+') value += term;
    else value -= term;
    skipSpaces(s, pos);
  }
  return value;
}

float parseTerm(const char* s, int *pos) {
  skipSpaces(s, pos);
  float value = parseFactor(s, pos);
  skipSpaces(s, pos);
  while (s[*pos] == '*' || s[*pos] == '/') {
    char op = s[(*pos)++];
    float factor = parseFactor(s, pos);
    if (op == '*') value *= factor;
    else value /= factor;
    skipSpaces(s, pos);
  }
  return value;
}

float parseFactor(const char* s, int *pos) {
  skipSpaces(s, pos);
  float base = parseUnary(s, pos);
  skipSpaces(s, pos);
  if (s[*pos] == '^') {
    (*pos)++;
    float exponent = parseFactor(s, pos);
    base = powf(base, exponent);
  }
  return base;
}

float parseUnary(const char* s, int *pos) {
  skipSpaces(s, pos);
  if (s[*pos] == '-') {
    (*pos)++;
    return -parseUnary(s, pos);
  }
  if (s[*pos] == '+') {
    (*pos)++;
    return parseUnary(s, pos);
  }
  return parsePrimary(s, pos);
}

// parse the argument of a function call: "(" expr ")"
static float parseParenArg(const char* s, int *pos) {
  skipSpaces(s, pos);
  float value = 0;
  if (s[*pos] == '(') {
    (*pos)++;
    value = parseExpression(s, pos);
    skipSpaces(s, pos);
    if (s[*pos] == ')') (*pos)++;
  }
  return value;
}

float parsePrimary(const char* s, int *pos) {
  skipSpaces(s, pos);
  float value = 0;

  if (s[*pos] == '(') {
    (*pos)++;
    value = parseExpression(s, pos);
    skipSpaces(s, pos);
    if (s[*pos] == ')') (*pos)++;
  }
  else if (matchWord(s, pos, "asin")) value = myAsin(parseParenArg(s, pos));
  else if (matchWord(s, pos, "acos")) value = myAcos(parseParenArg(s, pos));
  else if (matchWord(s, pos, "atan")) value = myAtan(parseParenArg(s, pos));
  else if (matchWord(s, pos, "sin"))  value = mySin(parseParenArg(s, pos));
  else if (matchWord(s, pos, "cos"))  value = myCos(parseParenArg(s, pos));
  else if (matchWord(s, pos, "sqrt")) value = mySqrt(parseParenArg(s, pos));
  else if (matchWord(s, pos, "log"))  value = myLog(parseParenArg(s, pos));
  else if (matchWord(s, pos, "ln"))   value = myLn(parseParenArg(s, pos));
  else if (matchWord(s, pos, "e^"))   value = myExp(parseParenArg(s, pos));
  else if (matchWord(s, pos, "pi"))   value = 3.141592653589793f;
  else if (s[*pos] == 'e') {
    value = 2.718281828459045f;
    (*pos)++;
  }
  else {
    char buffer[20];
    int i = 0;
    while (((s[*pos] >= '0' && s[*pos] <= '9') || s[*pos] == '.') && i < (int)sizeof(buffer) - 1) {
      buffer[i++] = s[(*pos)++];
    }
    buffer[i] = '\0';
    value = atof(buffer);
  }
  skipSpaces(s, pos);
  return value;
}

float evaluateExpression(const char* expr) {
  int pos = 0;
  return parseExpression(expr, &pos);
}

// -------------------------
// LCD Update Function
// -------------------------
void updateLCD(void) {
  lcd_clear();
  lcd_print(input);
}

// -------------------------
// Safe Append Helper
// -------------------------
bool appendSafe(const char* text) {
  size_t curLen = strlen(input);
  size_t addLen = strlen(text);
  if (curLen + addLen >= INPUT_SIZE - 1) return false;
  strcat(input, text);
  return true;
}

// -------------------------
// Button/Key Handling Functions
// -------------------------
void handleSpecial(char op) {
  if (op == '<') {
    const char* functions[] = {"sin(", "cos(", "e^(", "sqrt(", "asin(", "acos(", "atan(", "log(", "ln("};
    bool removed = false;
    for (int i = 0; i < 9; i++) {
      size_t flen = strlen(functions[i]);
      if (strlen(input) >= flen && strcmp(&input[strlen(input) - flen], functions[i]) == 0) {
        input[strlen(input) - flen] = '\0';
        removed = true;
        break;
      }
    }
    if (!removed && strlen(input) > 0) {
      input[strlen(input) - 1] = '\0';
    }
  }
  else if (op == '=') {
    char expr[INPUT_SIZE];
    strcpy(expr, input);
    float result = evaluateExpression(expr);
    char resultStr[32];
    dtostrf(result, 0, 3, resultStr);
    appendSafe("=");
    appendSafe(resultStr);
    updateLCD();
    _delay_ms(3000);
    input[0] = '\0';
  }
  else {
    size_t len = strlen(input);
    if (len < INPUT_SIZE - 1) {
      input[len] = op;
      input[len+1] = '\0';
    }
  }
  updateLCD();
}

// -------------------------
// Main Function
// -------------------------
int main(void) {
  lcd_init();
  lcd_clear();
  lcd_print("Calculator Ready");
  _delay_ms(1000);
  lcd_clear();

  while (1) {
    bool currentShiftState = shift_button_pressed();
    if (lastShiftState && !currentShiftState) {
      shiftActive = true;
    }
    lastShiftState = currentShiftState;

    bool currentExtraState = extra_button_pressed();
    if (lastExtraState && !currentExtraState) {
      extraActive = true;
    }
    lastExtraState = currentExtraState;

    for (uint8_t i = 0; i < 10; i++) {
      if (button_pressed(BUTTON_MASK(i))) {
        _delay_ms(50);
        if (button_pressed(BUTTON_MASK(i))) {
          while (button_pressed(BUTTON_MASK(i)));

          if (extraActive) {
            if (i < 5) {
              appendSafe(extraFuncs[i]);
              appendSafe("(");
            }
            else if (i == 5 || i == 6) {
              appendSafe(extraFuncs[i]);
            }
            else if (i == 7) {
              appendSafe("^");
            }
            else {
              size_t len = strlen(input);
              if (len < INPUT_SIZE - 1) {
                input[len] = normalMode[i];
                input[len+1] = '\0';
              }
            }
            extraActive = false;
          }
          else if (shiftActive) {
            if (i < 6) {
              handleSpecial(shiftOps[i]);
            }
            else {
              appendSafe(shiftFuncs[i - 6]);
              appendSafe("(");
            }
            shiftActive = false;
          }
          else {
            size_t len = strlen(input);
            if (len < INPUT_SIZE - 1) {
              input[len] = normalMode[i];
              input[len+1] = '\0';
            }
          }
          updateLCD();
        }
      }
    }

    _delay_ms(10);
  }

  return 0;
}