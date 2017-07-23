/*
 * File:   EEPROM.c
 * Author: True Administrator
 *
 * Created on February 22, 2017, 3:31 PM
 */

/* =============================================================================
This file contains the functions required to read and write to the EEPROM,
the registers used to store permanent data
================================================================================*/


#include <xc.h>
#include <stdio.h>
#include "main.h"

unsigned char ReadEE(unsigned char addressh, unsigned char address) {
	//read data from permanent memory
    EEADRH = addressh; //upperbits of the address
    EEADR = address; //first 8 bits of the address
    EECON1bits.EEPGD = 0;
    EECON1bits.CFGS = 0;
    EECON1bits.RD = 1;
    return EEDATA;
}

void WriteEE(unsigned char addressh, unsigned char address, unsigned char data) {
	//write data onto permanent memory so that user can see previous runs
    EECON1bits.WREN = 1; //enable writes
    EEADRH = addressh; //upper bits of data address
    EEADR = address; //lower bits of data address
    EEDATA = data; //data to be written
    EECON1bits.EEPGD = 0; //point to DATA memory
    EECON1bits.CFGS = 0; //access EEPROM
    INTCONbits.GIE = 0; //disable interrupts

    //required sequence
    EECON2 = 0x55;
    EECON2 = 0xAA;
    EECON1bits.WR = 1; //Set WR bit to begin write

    INTCONbits.GIE = 1; //enable interrupts
    while (EECON1bits.WR == 1) {} //waiting for write to complete
    EECON1bits.WREN = 0; //disable writes on write complete
}

void saveLog(unsigned char sortLog[]) {
    unsigned int address = 0;
    unsigned char addressh, addressl;
    while(ReadEE(((address >> 8) & 0xFF), (address & 0xFF)) != 0xFF) {
        address += 13; //we need to write to 13 addresses in memory to store the information for each run, so just address by 13
    } //look for a byte of memory that is unwritten to
    __lcd_clear();
    for(unsigned int i = 0; i < 13; i++) {
        addressh = (address >> 8) & 0xFF;
        addressl = address & 0xFF;
        __lcd_home();
        __delay_ms(10);
        printf("Saving Data... ");
        __delay_ms(10);
        WriteEE(addressh,addressl,sortLog[i]); //write to the EEPROM
        address++;
    }
    viewSavedLogs();
}

void viewSavedLogs(void) {
	//this code displays a prompt on the LCD Display to allow the user to see saved logs
    __lcd_clear();
    __delay_ms(1);
    printf(" View saved logs");
    __lcd_newline();
    printf("          C:Next");
    char key = keypadInterface();
    while(key != 'C'){
        key = keypadInterface(); //poll key C until it is pressed
    }
    __lcd_clear();
    __delay_ms(1);
    printf(" Press D to");
    __lcd_newline();
    printf("select a log");
    __delay_1s();
    unsigned char i = 0;
    unsigned char sortLog[13];
    while(1) {
        for(unsigned char j = 0; j < 13; j++) {
            sortLog[j] = ReadEE(0,i+j); //read data from the EEPROM and place it in the sortLog variable
        }
        __lcd_clear();
        __delay_ms(1);
        printf("%02x/%02x/%02x   %02x:%02x", sortLog[0],sortLog[1],sortLog[2],sortLog[4],sortLog[5]); //sortLog[0:6] contains data about the date and time of the run
        __lcd_newline();
        printf("B:Prev    C:Next");
        key = keypadInterface();
        if(key == 'C') {
            unsigned char temp = i + 13;
            if(ReadEE(0,temp) != 0xFF) { //this line checks to see if this is an empty spot in memory
                i = temp; //if it is, lets point to it with i
            }
        }
        else if(key == 'B' && i > 0) {
            i -= 13; //need to go back to view a previous run, so need to decrease address by 13
        }
        else if(key == 'D') {
            retrieveLog(1,sortLog); //view the sorting Log
        }
    }
}
