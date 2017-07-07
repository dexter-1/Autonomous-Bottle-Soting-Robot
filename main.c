/*
 * File:   main.c
 * Author: True Administrator
 *
 * Created on July 18, 2016, 12:11 PM
 */


#include <xc.h>
#include <stdio.h>
#include "configBits.h"
#include "constants.h"
#include "lcd.h"
//#include "I2C.h"

//global variables
const char keys[] = "123A456B789C*0#D"; 
unsigned char totalNumOfBottles = 0;
unsigned char YopNoCap = 0;
unsigned char YopCap = 0;
unsigned char EskaCap = 0;
unsigned char EskaNoCap = 0;
const char inputTime[7] = {  0x00, //Second
                            0x49, //Minute
                            0x05, //Hour
                            0x03, //Day of the Week 
                            0x28, //Day
                            0x3, //Month
                            0x17};//Year
unsigned char time[7];
unsigned char finalTime[7];
unsigned char sortTime[2];
int bins[4] = {0,1,2,3};
int currentBinPosition = 0;
int count = 0;
int totalCount = 0;

void main(void) {
    /* pin assignments:
     * RA2:RA5 - middle stepper motor - output
     * RC5:RC7,RC2 - bottom stepper motor - output
     * RD0 - signal to turn on top DC motor - output
     * RA5 - signal to see if a bottle lands in sorting chamber - input
     * RA4 - transparency check - input
     * RB0,RB2 - cap check - input
     */
    
    TRISA = 0b00000011;  //RA0,RA1 input, RA4,RA5 output
    TRISC = 0b01000000;   //all output mode except RC6
    TRISD = 0x00;   //All output mode
    TRISB = 0b11110111;   //Set keypad pins as input, RB0 as input, RB2 as input
    TRISE = 0x00;
    
    //initialize pins to 0
    LATA = 0x00;
    LATB = 0x00; 
    LATC = 0x00;
    LATD = 0x00;
    
//    //setting up interrupt timer
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
    printf("%02x/%02x/%02x   %02x:%02x", time[6],time[5],time[4],time[2],time[1]);
    __lcd_newline();
    if(ReadEE(0,0) != 0xFF) {
        printf("A:Logs   D:Start");
        __lcd_newline();
    }
    else if(ReadEE(0,0) == 0xFF) {
        printf("         D:Start");
        __lcd_newline();
    }
    while(1) {
        char key = keypadInterface();
        if (key == 'D') {
            read_time(); //sets timer
            operationMode();
        }
        else if(key == 'A' && ReadEE(0,0) != 0xFF) {
            viewSavedLogs();
        }
        else if(key == '1') {
            turnStepperMotor2(180);
        }
        else if(key == '4') {
            turnStepperMotor2(-180);
        }
        //test code for stepper motor
        else if(key == '2') {
            turnStepperMotor1(90);
        }
        else if(key == '5') {
            turnStepperMotor1(180);
        }
        else if(key == '8') {
            turnStepperMotor1(270);
        }
        else if(key == '0') {
            turnStepperMotor1(0);
        }
        else if(key == '6') {
            LATDbits.LATD0 = 1;
        }
        else if(key == '9') {
            LATDbits.LATD0 = 0;
        }

    }
//    unsigned char prevMins = time[1];
//    while(1) {
//        read_time();
//        if(prevMins != time[1]) {
//            __lcd_clear();
//            __delay_ms(1);
//            printf("%02x/%02x/%02x   %02x:%02x", time[6],time[5],time[4],time[2],time[1]);
//            __lcd_newline();
//            if(ReadEE(0,0) != 0xFF) {
//                printf("2:Logs   1:Start");
//                __lcd_newline();
//            }
//            else if(ReadEE(0,0) == 0xFF) {
//                printf("         1:Start");
//                __lcd_newline();
//            }
//        }
//        prevMins = time[1];
//        __delay_ms(1000);
//    }
    //unsigned int prevMinutes = time[1];
    //standby mode
    
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
            turnStepperMotor1(rotationAngle); //turn bottom motor
            turnStepperMotor2(-270); //open sensor tube
            turnStepperMotor2(270); //close sensor tube
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

//int rotationAmount(int bottleIdentity) {
//    int newBinPosition;
//    int rotation;
//    if(binAssigned(bins,bottleIdentity) == 0) {
//        newBinPosition = findClosestBin(bins);
//        bins[newBinPosition] = bottleIdentity;
//    }
//    else {
//        newBinPosition = findAssignedBin(bins,bottleIdentity);
//    }
//    rotation = newBinPosition - currentBinPosition;
//    currentBinPosition = newBinPosition;
////    if(abs(rotation) == 3) {
////        return ((rotation % 2)*90*-1);
////    }
////    else {
////        return rotation*90;
////    }
//    return abs(rotation*90);
//}

int rotationAmount(int bottleIdentity) {
    int counter = 0;
    while(bins[currentBinPosition] != bottleIdentity) {
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

//int binAssigned(int bins[], int bottleIdentity) {
//    for(unsigned int i = 0; i < 4; i++) {
//        if(bins[i] == bottleIdentity) 
//            return 1;
//    }
//    return 0;
//}

//int findAssignedBin(int bins[], int bottleIdentity) {
//    for(unsigned int i = 0; i < 4; i++) {
//        if(bins[i] == bottleIdentity) 
//            return i;
//    }
//    return -1;
//}
//
//int findClosestBin(int bins[]) {
//    int closestBin;
//    int distance = 4;
//    int temp;
//    for(unsigned int i = 0; i < 4; i++) {
//        if(bins[i] == -1) {
//            temp = abs(currentBinPosition - i);
//            if(temp == 3) {
//                if(distance > 1) {
//                    distance = 1;
//                    closestBin = i;
//                }      
//            }
//            else {
//                if (temp < distance) {
//                    distance = temp;
//                    closestBin = i;
//                }
//            }
//        }
//    }
//    return closestBin;
//}
//
//int abs(int value) {
//    if(value > 0) {
//        return value;
//    }
//    else {
//        return value*-1;
//    }
//}
//
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
        sortLog[0] = finalTime[6];
        sortLog[1] = finalTime[5];
        sortLog[2] = finalTime[4];
        sortLog[3] = finalTime[3];
        sortLog[4] = finalTime[2];
        sortLog[5] = finalTime[1];
        sortLog[6] = sortTime[1];
        sortLog[7] = sortTime[0];
        sortLog[8] = totalNumOfBottles;
        sortLog[9] = YopCap;
        sortLog[10] = YopNoCap;
        sortLog[11] = EskaCap;
        sortLog[12] = EskaNoCap;
        retrieveLog(0,sortLog);
    }
    //unsigned char key;
//    while(1) {
//        for(char k = 0; k < 21; k++) {
//            __lcd_shiftL();
//            __delay_ms(250);
//        }//key = keypadInterface();
//        __lcd_clear();
//        __delay_ms(1);
//        printf(" Sorting Complete! Retrieve bins below");
//        __lcd_newline();
//        printf("Press C to show sorting log ");
//        __delay_halfs();
//    }
//    di();
}

void retrieveLog(int mode, unsigned char sortLog[]) {
    int log = 0, newlog = 0; 
    //log is a variable, with values 0-6, with each value displaying a certain piece of information
    //sortLog[] is an array that contains all the information from the run
    unsigned char key; //which key is pressed?
    sortingLog(log,sortLog,mode); //function that displays the information on the log
    __delay_halfs();
    while(1) {
        if(newlog != log) {
            sortingLog(newlog,sortLog,mode); 
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

void retrieveLog2(int mode, unsigned char sortLog[]) {
    //if mode == 0, this is the log of the run that has just been completed
    //if mode == 1, this is the log that has been previously saved
    int log = 0, newlog = 0;
    unsigned char key;
    sortingLog(log,sortLog,mode);
    __delay_ms(250);
    while(1) {
        if(newlog != log) {
            sortingLog(newlog,sortLog,mode);
        }
        log = newlog;
        key = keypadInterface();
        if(key == 'C' && newlog < 6) {
            newlog++;
        }
        else if(key == 'B' && newlog > 0) {
            newlog--;
        }
        else if(key == 'C' && newlog == 6) {
            break;
        }
    }
}

void sortingLog(int log, unsigned char sortLog[], int mode) {
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

unsigned char ReadEE(unsigned char addressh, unsigned char address) {
    EEADRH = addressh;
    EEADR = address;
    EECON1bits.EEPGD = 0;
    EECON1bits.CFGS = 0;
    EECON1bits.RD = 1;
    return EEDATA;
}

void WriteEE(unsigned char addressh, unsigned char address, unsigned char data) {
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
        address += 13;
    } //look for a byte of memory that is unwritten to 
    __lcd_clear();
    for(unsigned int i = 0; i < 13; i++) {
        addressh = (address >> 8) & 0xFF;
        addressl = address & 0xFF;
        //debug code
        __lcd_home();
        __delay_ms(10);
        printf("Saving Data... ");
        __delay_ms(10);
        WriteEE(addressh,addressl,sortLog[i]);
        address++;
    }
    viewSavedLogs();
}

void viewSavedLogs(void) {
    __lcd_clear();
    __delay_ms(1);
    printf(" View saved logs");
    __lcd_newline();
    printf("          C:Next");
    char key = keypadInterface();
    while(key != 'C'){
        key = keypadInterface();
    }
    __lcd_clear();
    __delay_ms(1);
    printf(" Press D to");
    __lcd_newline();
    printf("select a log");
    __delay_1s();
    __delay_1s();
    unsigned char i = 0;
    unsigned char sortLog[13];
    while(1) {
        for(unsigned char j = 0; j < 13; j++) {
            sortLog[j] = ReadEE(0,i+j);
        }
        __lcd_clear();
        __delay_ms(1);
        printf("%02x/%02x/%02x   %02x:%02x", sortLog[0],sortLog[1],sortLog[2],sortLog[4],sortLog[5]);
        __lcd_newline();
        printf("B:Prev    C:Next");
        key = keypadInterface();
        if(key == 'C') {
            unsigned char temp = i + 13;
            if(ReadEE(0,temp) != 0xFF) {
                i = temp;
            }
        }
        else if(key == 'B' && i > 0) {
            i -= 13;
        }
        else if(key == 'D') {
            retrieveLog2(1,sortLog);
        }
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

//void turnStepperMotor1(int angle) {
//    //black, red, yellow, white (clockwise)
//    float factor; 
//    float rotation;
//    float temp;
//    temp = angle;
//    factor = temp/360;
//    rotation = fabs(12*factor);//fabs(1040*factor);
//    unsigned int i = 0; 
//    while(i < rotation) {
//        if(angle >= 0) {
//            //rotate clockwise
//            LATC = 0b10000000;//LATC = 0b11000000;
//            __delay_ms(10);
//            LATC = 0b01000000;//LATC = 0b01100000;
//            __delay_ms(10);
//            LATC = 0b00100000;//LATC = 0b00100100;
//            __delay_ms(10);
//            LATC = 0b00000100;//LATC = 0b10000100;
//            __delay_ms(10);
//        } 
//        else {
//            //rotate counterclockwise 
//            LATC = 0b00000100;//LATC = 0b10000100;
//            __delay_ms(10);
//            LATC = 0b00100000;//LATC = 0b00100100;
//            __delay_ms(10);
//            LATC = 0b01000000;//LATC = 0b01100000;
//            __delay_ms(10);
//            LATC = 0b10000000;//LATC = 0b11000000;
//            __delay_ms(10);
//        }
//        i++;
//    }
//}

void turnStepperMotor1(int angle) {
    int counter;
    counter = angle / 90;
    if(counter == 0) {
        return;
    }
    if(counter > 0) {
        PORTCbits.RC7 = 1;
    }
    while(PORTCbits.RC6 == 0);
    while(counter > 0) {
        if(PORTCbits.RC6 == 0) {
            if(counter == 1) {
                PORTCbits.RC7 = 0;
                break;
            }
            counter--;
            __delay_ms(5);
            while(PORTCbits.RC6 == 0);
        }
    }
}


void turnStepperMotor2(int angle) {
    float factor; 
    float rotation;
    float temp;
    temp = angle;
    factor = temp/360;
    rotation = fabs(512*factor);
    unsigned int i = 0; 
    while(i < rotation) {
        if(angle >= 0) {
            //rotate clockwise
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
            //rotate counterclockwise 
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

void set_final_time(void) {
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

void calcSortTime(void) {
    unsigned char initialMinutes, finalMinutes;
    unsigned char initialSeconds, finalSeconds;
    
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

void interrupt IRQ_HANDLER(void) {
    if(TMR0IF) { //timer interrupt flag
        TMR0 = 0; //reset TIMER0 register after overflowing
        count++; //increase count value 
        if(count == 687) { //count value of 687, corresponding to 3 minutes 
            di();
            set_final_time();
            terminationMode();
        }
        TMR0IF = 0; //clear interrupt flag
    }
}