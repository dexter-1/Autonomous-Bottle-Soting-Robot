/*
 * File:   main.h
 * Author: Administrator
 *
 * Created on February 12, 2017, 7:31 PM
 */

 #ifndef MAIN_H
 #define MAIN_H

 #include "configBits.h"
 #include "constants.h"
 #include "EEPROM.h"
 #include "lcd.h"
 #include "motor.h"
 #include "timer.h"

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

 #endif /* main.h */
