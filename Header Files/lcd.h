/*
 * File:   lcd.h
 * Author: Administrator
 *
 * Created on August 12, 2016, 4:24 PM
 */

#ifndef LCD_H
#define	LCD_H

//LCD Functions
void lcdInst(char data);
void lcdNibble(char data);
void initLCD(void);

#define __delay_1s() for(char i=0;i<100;i++){__delay_ms(10);}
#define __delay_halfs() for(char i=0;i<50;i++){__delay_ms(10);}
#define __lcd_newline() lcdInst(0b11000000);
#define __lcd_clear() lcdInst(0x01);
#define __lcd_home() lcdInst(0b10000000);
#define __lcd_shiftL() lcdInst(0b00011000);
#define __bcd_to_num(num) (num & 0x0F) + ((num & 0xF0)>>4)*10

#endif	/* LCD_H */
