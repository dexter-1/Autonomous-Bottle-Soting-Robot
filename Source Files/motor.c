/*
 * File:   motor.c
 * Author: True Administrator
 *
 * Created on February 18, 2017, 2:26 PM
 */

/* =============================================================================
This file contains the functions to control the two motors on the robot
================================================================================*/

#include <xc.h>
#include <stdio.h>
#include "main.h"

void turnBottomMotor(int angle) {
	//code used to rotate DC motor and correct bin underneath sorting tube
	//PORT RC6 is a microswitch, 0 means the microswitch has come in contact with one bin
	//PORT RC7 is for the DC motor. 0 means turn motor off
    int counter;
    counter = angle / 90; //counter can take on the values of 0-3. 1 value of the count is 90 degrees
    if(counter == 0) {
        return;
    }
    if(counter > 0) {
        PORTCbits.RC7 = 1; //turn motor on
    }
    while(PORTCbits.RC6 == 0); //wait for the microswitch to be touched (i.e. a bin passes underneath the tub)
    while(counter > 0) {
        if(PORTCbits.RC6 == 0) {
            if(counter == 1) {
                PORTCbits.RC7 = 0; //turn motor off, we have put the right bin underneath the tub
                break;
            }
            counter--; //decrement counter by 1 (we have rotated the bottom platform by 90 degrees)
            __delay_ms(5);
            while(PORTCbits.RC6 == 0); //wait again for the microswitch to be touched
        }
    }
}


void turnStepperMotor(int angle) {
	//code used to turn stepper motor, allowing bottle to fall from sorting tube into the bin
    float factor;
    float rotation;
    float temp;
    temp = angle;
    factor = temp/360;
    rotation = fabs(512*factor); //calculate how long we need to execute the stepping sequence
    unsigned int i = 0;
    while(i < rotation) {
        if(angle >= 0) {
            //execute stepping sequence for the stepper motor to rotate clockwise
            LATA = 0b00110000;
            __delay_us(1500);
            LATA = 0b00011000;
            __delay_us(1500);
            LATA = 0b00001100;
            __delay_us(1500);
            LATA = 0b00100100;
            __delay_us(1500);
        }
        else {
            //execute stepping sequence for the stepper motor to rotate counterclockwise
            LATA = 0b00100100;
            __delay_us(1500);
            LATA = 0b00001100;
            __delay_us(1500);
            LATA = 0b00011000;
            __delay_us(1500);
            LATA = 0b00110000;
            __delay_us(1500);
        }
        i++;
    }
}
