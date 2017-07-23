/*
 * File:   constants.h
 * Author: Administrator
 *
 * Created on February 22, 2017, 4:49 PM
 */

#ifndef EEPROM_H
#define EEPROM_H

//EEPROM functions
unsigned char ReadEE(unsigned char, unsigned char);
void WriteEE(unsigned char, unsigned char, unsigned char);
void saveLog(unsigned char [13]);
void viewSavedLogs(void);

#endif /* EEPROM.h */
