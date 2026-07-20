#ifndef LCD_H
#define LCD_H

#include <avr/io.h>
#include <stdint.h>

// Pin mapping - change these to match your wiring.
// Standard 4-bit HD44780 interface: RS, EN, D4-D7.
// RW is assumed tied to GND (write-only mode).
#define LCD_RS_PORT   PORTD
#define LCD_RS_DDR    DDRD
#define LCD_RS_PIN    PD2

#define LCD_EN_PORT   PORTD
#define LCD_EN_DDR    DDRD
#define LCD_EN_PIN    PD3

#define LCD_D4_PORT   PORTD
#define LCD_D4_DDR    DDRD
#define LCD_D4_PIN    PD4

#define LCD_D5_PORT   PORTD
#define LCD_D5_DDR    DDRD
#define LCD_D5_PIN    PD5

#define LCD_D6_PORT   PORTD
#define LCD_D6_DDR    DDRD
#define LCD_D6_PIN    PD6

#define LCD_D7_PORT   PORTD
#define LCD_D7_DDR    DDRD
#define LCD_D7_PIN    PD7

// Display geometry
#define LCD_COLS 16
#define LCD_ROWS 2

void lcd_init(void);
void lcd_clear(void);
void lcd_home(void);
void lcd_command(uint8_t cmd);
void lcd_data(uint8_t data);
void lcd_print(const char *str);
void lcd_set_cursor(uint8_t col, uint8_t row);

#endif
