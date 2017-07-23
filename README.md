# Autonomous Bottle Sorting Robot Project

## Goal
The goal of this project is to enhance the recycling process by building a robot to autonomously sort bottles of different types. This robot was programmed in C using a PIC18F4620 Microcontroller. This robot was designed to sort bottles of four different categories:
 * Transparent Water Bottle with a Cap
 * Transparent Water Bottle without a Cap
 * Yoplait Yogurt bottle with a Cap
 * Yoplait yogurt bottle without a cap

## What the robot does
A bin of bottles containing a random amount of each type of bottle is placed into the loading area of the robot. The user will press the `D` button the keypad and the robot will begin sorting the bottles autonomously. Once the sorting has completed, the operator will be able to retrieve the 4 bins, with each bin containing one type of bottle. The user can then navigate a User interface to retrieve information on the run, including how many bottles were sorted, how long it took to sort the bottles, and how many of each bottle was sorted.

## Specifications
The frame of the robot was mostly made of scrap pieces of wood and aluminum beams. The main parts of the robot which make it operate include:
 * PIC18F4620 Microcontroller
 * PIC developer board, including a 2X20 I/O bus to control peripherals
 * 2 high torque DC motors
 * 28BJY-48 stepper motor
 * 16X2 LCD display for the User interface
 * 4x4 keypad to operate the machine and navigate User Interface
 * IR Receiver and Emitter Sensor Pair

## How it works

![](/Images/Robot.png)

Above is an image of the robot. The three coloured brackets in the picture represent the three main components of the robot. The green bracket shows the loading area.

![](/Images/loading.png)

Bottles are loaded in this area. When the user presses the start button, a DC motor is activated which rotates the platform and brings the bottles one by one onto the sorting tube.

![](/Images/SortingTube.png)

The above image shows the sorting tube. The bottles fall in here horizontally. There is an IR emitter at the bottom side of the tube and an IR receiver at the top to determine the transparency of the bottle. On the left and right side of the tube are IR Receiver and Emitter pairs to determine if a cap is one the bottle. If nothing is detected by any of the left or right sensors, then the bottle has no cap. The PIC board receives signals from these sensors and the code processes these signals to correctly classify the bottles. The code then determines how much to rotate the bottom platform (show inside the yellow brackets in the first picture of the robot) to place the correct bin underneath the tube. The code then applies the appropriate stepping sequence to turn a stepper motor which opens the platform and allows the bottle to fall inside the bin. This process is repeated for all bottles.

Once sorting has completed, the user can navigate the User Interface to see how long the sorting took and the number of bottles of each type of bottle. The user also has the option to store the information of this run to be viewed for later. Below shows an image of the user interface:

![](/Images/UI.png)

This repository contains all of the code required to execute the robot.
