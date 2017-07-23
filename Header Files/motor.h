/*
 * File:   constants.h
 * Author: Administrator
 *
 * Created on February 18, 2017, 6:23 PM
 */

#ifndef MOTOR_H
#define MOTOR_H

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

 #endif /* motor.h */
