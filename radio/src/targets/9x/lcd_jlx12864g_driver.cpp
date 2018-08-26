/*
 * Copyright (C) OpenTX
 *
 * Based on code named
 *   th9x - http://code.google.com/p/th9x 
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

void lcdSendCtl(uint8_t val)
{
  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_CS1);
#if defined(LCD_MULTIPLEX)
  DDRA = 0xFF; // Set LCD_DAT pins to output
#endif
  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_A0);
  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_RnW);
  PORTA_LCD_DAT = val;
  PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_E);
  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_E);
  PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_A0);
#if defined(LCD_MULTIPLEX)
  DDRA = 0x00; // Set LCD_DAT pins to input
#endif
  PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_CS1);
}

const static pm_uchar lcdInitSequence[] PROGMEM =
{
   0xE2, //Initialize the internal functions
   0x2C, //升压1
   0x2E, //升压2
   0x2F, //升压3
   0x24, //粗条对比度, 0x20~0x27
   0x81, //微调对比度
   0x1C, //微调对比度的值, 0x00~0x3f
   0xA2, //1/9偏压比(bias)
   0xC0, //行扫描顺序, 从上到下
   0xA1, //列扫描顺序, 从左到右
   0xAF, //开启显示
};

void lcdInit()
{
  LCD_LOCK();
  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_RES); // LCD reset
  _delay_us(20);
  PORTC_LCD_CTRL |= (1<<OUT_C_LCD_RES); // LCD normal operation
  _delay_us(1500);
  for (uint8_t i=0; i<DIM(lcdInitSequence); i++) {
    lcdSendCtl(pgm_read_byte(&lcdInitSequence[i])) ;
    _delay_us(5);
  }
  g_eeGeneral.contrast = 0x22;
  LCD_UNLOCK();
}

void lcdSetRefVolt(uint8_t val)
{
  LCD_LOCK();
  lcdSendCtl(0x81);
  lcdSendCtl(val);
  LCD_UNLOCK();
}

void lcdRefresh()
{
  LCD_LOCK();
  uint8_t * p = displayBuf;
  for (uint8_t y=0; y < 8; y++) {
    lcdSendCtl(0x01);
    lcdSendCtl(0x10); // Column addr 0
    lcdSendCtl( y | 0xB0); //Page addr y
    PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_CS1);
#if defined(LCD_MULTIPLEX)
    DDRA = 0xFF; // Set LCD_DAT pins to output
#endif
    PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_A0);
    PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_RnW);
    for (coord_t x=LCD_W; x>0; --x) {
      PORTA_LCD_DAT = *p++;
      PORTC_LCD_CTRL |= (1<<OUT_C_LCD_E);
      PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_E);
    }
    PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_A0);
    PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_CS1);
  }
  LCD_UNLOCK();
}
