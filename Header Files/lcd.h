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

//main program functions
void standbyMode(void);
void reset(void);
void terminationMode(void);
void operationMode(void);
void sortingLog(int,unsigned char[13],int);
char keypadInterface(void);
void retrieveLog(int,unsigned char[13]);
void retrieveLog2(int,unsigned char[13]);
void interrupt IRQ_HANDLER(void);
int bottleIdentifier(void);

//timer functions
void set_time(void);
void read_time(void);
void set_final_time(void);
void calcSortTime(void);

//I2C functions
void I2C_Master_Init(const unsigned long c);
void I2C_Master_Write(unsigned d);
unsigned char I2C_Master_Read(unsigned char a);

//motor functions
int rotationAmount(int);
int binAssigned(int [4], int);
int findAssignedBin(int[4],int);
int findClosestBin(int [4]);
int abs(int);
float fabs(float);
void turnStepperMotor1(int);
void turnStepperMotor2(int);
void continuousRotate(void);
void turnStepperMotorTest(int);


//EEPROM functions
unsigned char ReadEE(unsigned char, unsigned char);
void WriteEE(unsigned char, unsigned char, unsigned char);
void saveLog(unsigned char [13]);
void viewSavedLogs(void);

//motor speed
void turnMotorSpeed1(void);
void turnMotorSpeed2(void);


#define __delay_1s() for(char i=0;i<100;i++){__delay_ms(10);}
#define __delay_halfs() for(char i=0;i<50;i++){__delay_ms(10);}
#define __lcd_newline() lcdInst(0b11000000);
#define __lcd_clear() lcdInst(0x01);
#define __lcd_home() lcdInst(0b10000000);
#define __lcd_shiftL() lcdInst(0b00011000);
#define __bcd_to_num(num) (num & 0x0F) + ((num & 0xF0)>>4)*10

#endif	/* LCD_H */

