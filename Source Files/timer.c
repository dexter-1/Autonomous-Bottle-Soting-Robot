/*
 * File:   timer.c
 * Author: True Administrator
 *
 * Created on February 22, 2017, 3:31 PM
 */

/* =============================================================================
This file contains the functions dealing with timing operations on the robot,
including display the real time date and time for the run and how long the
run takes
================================================================================*/

#include <xc.h>
#include <stdio.h>
#include "main.h"

void set_time(void){
    I2C_Master_Start(); //Start condition
    I2C_Master_Write(0b11010000); //7 bit RTC address + Write
    I2C_Master_Write(0x00); //Set memory pointer to seconds
    for(char i=0; i<7; i++){
        I2C_Master_Write(inputTime[i]);
    }
    I2C_Master_Stop(); //Stop condition
}

void read_time(void) {
    //Reset RTC memory pointer
    I2C_Master_Start(); //Start condition
    I2C_Master_Write(0b11010000); //7 bit RTC address + Write
    I2C_Master_Write(0x00); //Set memory pointer to seconds
    I2C_Master_Stop(); //Stop condition

    //Read Current Time
    I2C_Master_Start();
    I2C_Master_Write(0b11010001); //7 bit RTC address + Read
    for(unsigned char i=0;i<0x06;i++){
        time[i] = I2C_Master_Read(1);
    }
    time[6] = I2C_Master_Read(0);       //Final Read without ack
    I2C_Master_Stop();
}

void set_final_time(unsigned char finalTime[], unsigned char sortTime[]) {
    //Reset RTC memory pointer
    I2C_Master_Start(); //Start condition
    I2C_Master_Write(0b11010000); //7 bit RTC address + Write
    I2C_Master_Write(0x00); //Set memory pointer to seconds
    I2C_Master_Stop(); //Stop condition

    //Read Current Time
    I2C_Master_Start();
    I2C_Master_Write(0b11010001); //7 bit RTC address + Read
    for(unsigned char i=0;i<0x06;i++){
        finalTime[i] = I2C_Master_Read(1);
    }
    finalTime[6] = I2C_Master_Read(0);       //Final Read without ack
    I2C_Master_Stop();

    calcSortTime();
}

void calcSortTime(unsigned char finalTime[], unsigned char sortTime[]) {
    unsigned char initialMinutes, finalMinutes;
    unsigned char initialSeconds, finalSeconds;

	//calculate the time required to sort the bottle by subtracting the
	//final time from the start time

	//time stored in the Real Time CLock (RTC Module) is in BCD. Need to convert to decimal first
    finalMinutes = (finalTime[1] >> 4)*10 + (finalTime[1] & 0x0F); //convert BCD to decimal number
    finalSeconds = (finalTime[0] >> 4)*10 + (finalTime[0] & 0x0F); //convert BCD to decimal number

    initialMinutes = (time[1] >> 4)*10 + (time[1] & 0x0F); //convert BCD to decimal number
    initialSeconds = (time[0] >> 4)*10 + (time[0] & 0x0F); //convert BCD to decimal number

    if(finalSeconds < initialSeconds) { //in this case, borrow 1 from the minutes and add 60 to seconds
        finalMinutes = finalMinutes - 1;
        finalSeconds = finalSeconds + 60;
        sortTime[0] = finalSeconds - initialSeconds;
        sortTime[1] = finalMinutes - initialMinutes;
    }
    else { //else just subtract minutes and seconds
        sortTime[0] = finalSeconds - initialSeconds;
        sortTime[1] = finalMinutes - initialMinutes;
    }
}
