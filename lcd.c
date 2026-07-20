#include "lcd.h"
#include <util/delay.h>

// HD44780 command bits
#define LCD_CLEAR_DISPLAY   0x01
#define LCD_RETURN_HOME     0x02
#define LCD_ENTRY_MODE_SET  0x04
#define LCD_DISPLAY_CTRL    0x08
#define LCD_FUNCTION_SET    0x20
#define LCD_SET_DDRAM_ADDR  0x80

#define LCD_ENTRY_LEFT      0x02
#define LCD_DISPLAY_ON      0x04
#define LCD_CURSOR_OFF      0x00
#define LCD_BLINK_OFF       0x00
#define LCD_4BIT_MODE       0x00
#define LCD_2LINE           0x08
#define LCD_5x8DOTS         0x00

static void pulse_enable(void) {
  LCD_EN_PORT |= (1 << LCD_EN_PIN);
  _delay_us(1);
  LCD_EN_PORT &= ~(1 << LCD_EN_PIN);
  _delay_us(100);
}

static void write_nibble(uint8_t nibble) {
  if (nibble & 0x01) LCD_D4_PORT |= (1 << LCD_D4_PIN); else LCD_D4_PORT &= ~(1 << LCD_D4_PIN);
  if (nibble & 0x02) LCD_D5_PORT |= (1 << LCD_D5_PIN); else LCD_D5_PORT &= ~(1 << LCD_D5_PIN);
  if (nibble & 0x04) LCD_D6_PORT |= (1 << LCD_D6_PIN); else LCD_D6_PORT &= ~(1 << LCD_D6_PIN);
  if (nibble & 0x08) LCD_D7_PORT |= (1 << LCD_D7_PIN); else LCD_D7_PORT &= ~(1 << LCD_D7_PIN);
  pulse_enable();
}

static void write_byte(uint8_t value, uint8_t is_data) {
  if (is_data) LCD_RS_PORT |= (1 << LCD_RS_PIN);
  else         LCD_RS_PORT &= ~(1 << LCD_RS_PIN);

  write_nibble(value >> 4);
  write_nibble(value & 0x0F);
  _delay_us(50);
}

void lcd_command(uint8_t cmd) {
  write_byte(cmd, 0);
  if (cmd == LCD_CLEAR_DISPLAY || cmd == LCD_RETURN_HOME) {
    _delay_ms(2);
  }
}

void lcd_data(uint8_t data) {
  write_byte(data, 1);
}

void lcd_init(void) {
  LCD_RS_DDR |= (1 << LCD_RS_PIN);
  LCD_EN_DDR |= (1 << LCD_EN_PIN);
  LCD_D4_DDR |= (1 << LCD_D4_PIN);
  LCD_D5_DDR |= (1 << LCD_D5_PIN);
  LCD_D6_DDR |= (1 << LCD_D6_PIN);
  LCD_D7_DDR |= (1 << LCD_D7_PIN);

  _delay_ms(50);

  // HD44780 init sequence: forces 8-bit mode 3 times, then switches to 4-bit
  LCD_RS_PORT &= ~(1 << LCD_RS_PIN);
  write_nibble(0x03);
  _delay_ms(5);
  write_nibble(0x03);
  _delay_us(150);
  write_nibble(0x03);
  write_nibble(0x02); // switch to 4-bit mode

  lcd_command(LCD_FUNCTION_SET | LCD_4BIT_MODE | LCD_2LINE | LCD_5x8DOTS);
  lcd_command(LCD_DISPLAY_CTRL | LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF);
  lcd_command(LCD_CLEAR_DISPLAY);
  lcd_command(LCD_ENTRY_MODE_SET | LCD_ENTRY_LEFT);
}

void lcd_clear(void) {
  lcd_command(LCD_CLEAR_DISPLAY);
}

void lcd_home(void) {
  lcd_command(LCD_RETURN_HOME);
}

void lcd_set_cursor(uint8_t col, uint8_t row) {
  static const uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};
  if (row >= LCD_ROWS) row = LCD_ROWS - 1;
  lcd_command(LCD_SET_DDRAM_ADDR | (col + row_offsets[row]));
}

void lcd_print(const char *str) {
  while (*str) {
    lcd_data((uint8_t)*str++);
  }
}
