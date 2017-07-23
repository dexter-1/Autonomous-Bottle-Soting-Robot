/*
 * File:   main.c
 * Author: True Administrator
 *
 * Created on July 18, 2016, 12:11 PM
 */


#include <xc.h>
#include <stdio.h>
#include "main.h"

//global variables
const char keys[] = "123A456B789C*0#D"; //all of the keys on the Keypad interface
unsigned char totalNumOfBottles = 0; //variable keeps track of all the bottles sorted
unsigned char YopNoCap = 0; //variable keeps track of all the Yoplait bottles with no cap sorted
unsigned char YopCap = 0; //variable keeps track of all Yoplait bottles with a cap sorted
unsigned char EskaCap = 0; //variable keeps track of all Eska (transparent) bottles with a cap sorted
unsigned char EskaNoCap = 0; //variable keeps track of all Eska (transparent) bottles without a cap sorted
const char inputTime[7] = {  0x00, //Second
                            0x49, //Minute
                            0x05, //Hour
                            0x03, //Day of the Week
                            0x28, //Day
                            0x3, //Month
                            0x17};//Year
unsigned char time[7]; //array to hold information on the current date and time
unsigned char finalTime[7];
unsigned char sortTime[2]; //array to hold the minutes and seconds value of the time required to sort
int currentBinPosition = 0; //variable to keep track of which bin is underneath the sorting tube

//variables required for the interrupt timer
int count = 0;
int totalCount = 0;

void main(void) {
    /* pin assignments on the PIC Board:
     * RA2:RA5 - middle stepper motor - output
     * RC5:RC7,RC2 - bottom stepper motor - output
     * RD0 - signal to turn on top DC motor - output
     * RA5 - signal to see if a bottle lands in sorting chamber - input
     * RA4 - transparency check - input
     * RB0,RB2 - cap check - input
     */

	//initialize input and output ports
    TRISA = 0b00000011;  //set RA0,RA1 input, RA4,RA5 output
    TRISC = 0b01000000;   //all output mode except RC6
    TRISD = 0x00;   //All output mode
    TRISB = 0b11110111;   //Set keypad pins as input, RB0 as input, RB2 as input
    TRISE = 0x00;

    //initialize pins to 0
    LATA = 0x00;
    LATB = 0x00;
    LATC = 0x00;
    LATD = 0x00;

//    //setting up Real Time Clock. Uncomment and execute once to set time on display, then comment out
//    T0CS = 0; //Set timer0 to use internal clock
//    T0SE = 0; //count on rising edge
//    PSA = 0;
//    T0PS2 = 1;
//    T0PS1 = 1;
//    T0PS0 = 1; //max prescaler
//    TMR0IE = 1;

    ADCON0 = 0x00;  //Disable ADC
    ADCON1 = 0xFF;  //Set PORTB to be digital instead of analog default
    CMCON = 0x07; //set PORTA to digital

    nRBPU = 0;
    //INT1IE = 1; //set up interrupt bit
    I2C_Master_Init(10000); //Initialize I2C Master with 100KHz clock
    initLCD(); //initialize LCD
    di();

    lcdInst(0b00001100); //turn off the blinking cursor
    //set_time();
    read_time();
    printf("%02x/%02x/%02x   %02x:%02x", time[6],time[5],time[4],time[2],time[1]); //display time onto the UI display
    __lcd_newline();
    if(ReadEE(0,0) != 0xFF) { //only display this is there is something in permanent memory to display
        printf("A:Logs   D:Start");
        __lcd_newline();
    }
    else if(ReadEE(0,0) == 0xFF) {
        printf("         D:Start");
        __lcd_newline();
    }
    while(1) { //poll the keypad and perform action based on what user presses on keypad
        char key = keypadInterface();
        if (key == 'D') { //press key D on the keypad to start the operation
            read_time(); //sets timer
            operationMode(); //begin sorting
        }
        else if(key == 'A' && ReadEE(0,0) != 0xFF) { //press A to view Saved Logs of previous runs
            viewSavedLogs();
        }
        else if(key == '6') {
            LATDbits.LATD0 = 1;
        }
        else if(key == '9') {
            LATDbits.LATD0 = 0;
        }

    }
    return;
}

void operationMode(void) {
    ei();
    __lcd_clear();
    __delay_ms(1);
    printf(" ..Now Sorting..");
    __lcd_newline();

    //Sorting has begun
    LATDbits.LATD0 = 1; //turn on top motor (motor is controlled by pin RD0)

    int bottleIdentity;
    int rotationAngle;

    //code to sort bottles
    while(totalNumOfBottles < 10) { //termination condition
        if(PORTAbits.RA1 == 0) {
            LATDbits.LATD0 = 0; //turn off loading motor
            __delay_ms(500); //delay to allow the bottle to fall and align with the sensors
            bottleIdentity = bottleIdentifier(); //bottle identification
            rotationAngle = rotationAmount(bottleIdentity); //determine rotation amount
            turnBottomMotor(rotationAngle); //turn bottom motor
            turnStepperMotor(-270); //open sensor tube
            turnStepperMotor(270); //close sensor tube
            LATDbits.LATD0 = 1; //turn on loading motor
        }
    }
    //if we get here, sorting has completed


    di();
    set_final_time(); //stop the timer
    terminationMode();
    while(1);
}

int bottleIdentifier(void) {
    /*bottle identification legend:
     * 0: Yop with Cap
     * 1: Yop with no cap
     * 2: Eska with Cap
     * 3: Eska no Cap
     */
    //0: detection, 1: no detection
    totalNumOfBottles++; //increment the total number of bottles by 1
    if(PORTAbits.RA0 == 0) { //break beam sensor: this is the opaque case
        if(PORTBbits.RB0 | PORTBbits.RB2 == 1) { //at least one sensor detects nothing
            YopNoCap++;
            return 1;
        }
        else if(PORTBbits.RB0 | PORTBbits.RB2 == 0) {
            YopCap++;
            return 0;
        }
    }
    else if(PORTAbits.RA0 == 1) { //break beam sensor: this is the transparent case
        if(PORTBbits.RB0 | PORTBbits.RB2 == 1) {
            EskaNoCap++;
            return 3;
        }
        else if(PORTBbits.RB0 | PORTBbits.RB2 == 1) {
            EskaNoCap++;
            return 2;
        }
    }
}

int rotationAmount(int bottleIdentity) {
	//this function is used to determine how much to rotate bottom platform based on the identity of bottle
    int counter = 0;
    while(currentBinPosition != bottleIdentity) {
        if(currentBinPosition == 3) {
            currentBinPosition = 0;
        }
        else {
            currentBinPosition++;
        }
        counter++;
    }
    return counter*90;
}

float fabs(float value) {
    if(value > 0) {
        return value;
    }
    else {
        return value*-1;
    }
}

void terminationMode(void) {
    di();
    __lcd_clear();
    __delay_ms(1);
    printf(" Complete!");
    __lcd_newline();
    printf("Press C for log");
    LATDbits.LATD0 = 0; //turn off top motor
    __delay_halfs();
    char key = keypadInterface();
    while (key != 'C') {
        key = keypadInterface();
    }
    if(key == 'C') {
        unsigned char sortLog[13];
		//store all of the information from the run into the sortingLog variable
        sortLog[0] = finalTime[6]; //
        sortLog[1] = finalTime[5];
        sortLog[2] = finalTime[4];
        sortLog[3] = finalTime[3];
        sortLog[4] = finalTime[2];
        sortLog[5] = finalTime[1];
        sortLog[6] = sortTime[1]; //minutes of the final sort time
        sortLog[7] = sortTime[0]; //seconds of the final sort time
        sortLog[8] = totalNumOfBottles; //total number of bottles sorted
        sortLog[9] = YopCap; //total number of Yoplait bottles with a cap that has been sorted
        sortLog[10] = YopNoCap; //total number of Yoplait bottles without a cap that has been sorted
        sortLog[11] = EskaCap; //total number of Eska (transparent) bottles with a cap that has been sorted
        sortLog[12] = EskaNoCap; //total number of Eska (transparent) bottles with a cap that has been sorted
        retrieveLog(0,sortLog);
    }
}

void retrieveLog(int mode, unsigned char sortLog[]) {
    int log = 0, newlog = 0;
    //log is a variable, with values 0-6, with each value displaying a certain piece of information
    //sortLog[] is an array that contains all the information from the run
    unsigned char key; //which key is pressed?
    sortingLog(log,sortLog,mode); //function that displays the information on the log
    __delay_halfs();
    while(1) { //while loop is used to allow user to navigate the menu
        if(newlog != log) {
            sortingLog(newlog,sortLog,mode); //display the appropriate log
        }
        log = newlog;
        key = keypadInterface(); //which key is pressed?
        if(key == 'C' && newlog < 6) {
            newlog++; //if C is pressed, move to the next piece of information
        }
        else if(key == 'B' && newlog > 0) {
            newlog--; //if B is pressed, move to the previous piece of information
        }
        else if(key == 'C' && newlog == 6) {
            break; //there are only 6 piece of information, so don't do anything if C is pressed
        }
    }
}

void sortingLog(int log, unsigned char sortLog[], int mode) {
	//this is the navigation menu after execution has completed
	//user can see how much time the sorting took and number of each type of Bottle
    switch(log) {
        case(0):
            __lcd_clear();
            __delay_ms(1);
            printf(" Sort Time: %01d:%02d ", sortLog[6], sortLog[7]); //time in M:SS
            __lcd_newline();
            printf("          C:Next");
            break;
        case(1):
            __lcd_clear();
            __delay_ms(1);
            printf(" Bottle Total:%d", sortLog[8]); //sortLog[8] contains totalNumOfBottles
            __lcd_newline();
            printf("B:Back    C:Next");
            break;
        case(2):
            __lcd_clear();
            __delay_ms(1);
            printf(" Yop with cap:%d", sortLog[9]); //sortLog[9] contains total number of Yop with Cap
            __lcd_newline();
            printf("B:Back    C:Next");
        break;
        case(3):
            __lcd_clear();
            __delay_ms(1);
            printf(" Yop no cap:%d", sortLog[10]); //sortLog[10] contains total of number of Yop without Cap
            __lcd_newline();
            printf("B:Back    C:Next");
        break;
        case(4):
            __lcd_clear();
            __delay_ms(1);
            printf(" Eska with cap:%d", sortLog[11]); //sortLog[11] contains total of number of Eska with Cap
            __lcd_newline();
            printf("B:Back    C:Next");
        break;
        case(5):
            __lcd_clear();
            __delay_ms(1);
            printf(" Eska no cap:%d", sortLog[12]);//sortLog[12] contains total of number of Eska without Cap
            __lcd_newline();
            printf("B:Back    C:Next");
        break;
        default: break;
    }
}

char keypadInterface(void) {
    while(PORTBbits.RB1 == 0){
            // RB1 is the interrupt pin, so if there is no key pressed, RB1 will be 0
            // the PIC will wait and do nothing until a key press is signaled
    }
    unsigned char keypress = (PORTB & 0xF0)>>4; // Read the 4 bit character code
    while(PORTBbits.RB1 == 1){
        // Wait until the key has been released
    }
    Nop();  //Apply breakpoint here because of compiler optimizations
    Nop();
    unsigned char temp = keys[keypress];
    return temp;
}

//this interrupt is used to terminate operation of the robot after 3 minutes
void interrupt IRQ_HANDLER(void) {
    if(TMR0IF) { //timer interrupt flag
        TMR0 = 0; //reset TIMER0 register after overflowing
        count++; //increase count value
        if(count == 687) { //count value of 687, corresponding to 3 minutes
            di();
            set_final_time(finalTime, sortTime);
            terminationMode();
        }
        TMR0IF = 0; //clear interrupt flag
    }
}
