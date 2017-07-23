/*
 * File:   constants.h
 * Author: Administrator
 *
 * Created on February 25, 2017, 12:02 PM
 */

#ifndef TIMER_H
#define	TIMER_H

//timer functions
void set_time(void);
void read_time(void);
void set_final_time(void);
void calcSortTime(void);

//I2C functions
void I2C_Master_Init(const unsigned long c);
void I2C_Master_Write(unsigned d);
unsigned char I2C_Master_Read(unsigned char a);

#endif /* timer.h */
