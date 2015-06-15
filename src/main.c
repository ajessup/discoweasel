// This is your second program to run on the LaunchPad
// You will debug this program as your Lab 4
// If both switches SW1 and SW2 are pressed, the LED should be blue
// If just SW1 switch is pressed,            the LED should be red
// If just SW2 switch is pressed,            the LED should be green
// If neither SW1 or SW2 is pressed,         the LED should be off

// 0.Documentation Section 
// main.c
// Runs on LM4F120 or TM4C123
// Lab4_IO, Inputs from PF4,PF0, output to PF3,PF2,PF1 (LED)
// Authors: Daniel Valvano, Jonathan Valvano and Ramesh Yerraballi
// Date: December 28, 2014

// LaunchPad built-in hardware
// SW1 left switch is negative logic PF4 on the Launchpad
// SW2 right switch is negative logic PF0 on the Launchpad
// red LED connected to PF1 on the Launchpad
// blue LED connected to PF2 on the Launchpad
// green LED connected to PF3 on the Launchpad

#include "gpio.h"

// 2. Declarations Section
//   Global Variables
unsigned long SW1,SW2;  // input from PF4,PF0
unsigned long Out;      // outputs to PF3,PF2,PF1 (multicolor LED)



// 3. Subroutines Section
// MAIN: Mandatory for a C Program to be executable
int main(void){    
  //TExaS_Init(SW_PIN_PF40,LED_PIN_PF321); 
  // TExaS_Init initializes the real board grader for lab 4
  PortF_Init();        // Call initialization of port PF4, PF3, PF2, PF1, PF0    
  //EnableInterrupts();  // The grader uses interrupts
  while(1){
    SW1 = GPIO_PORTF_DATA_R&0x01;     // read PF0 into SW1
    SW2 = GPIO_PORTF_DATA_R&0x10;     // read PF4 into SW2
    if(SW1&&SW2){                     // both pressed
      GPIO_PORTF_DATA_R = 0x04;       // LED is blue
    } else{                           
      if(SW1&&(!SW2)){                // just SW1 pressed
        GPIO_PORTF_DATA_R = 0x02;     // LED is red
      } else{                        
        if((!SW1)&&SW2){              // just SW2 pressed
          GPIO_PORTF_DATA_R = 0x08;   // LED is green
        }else{                        // neither switch
          GPIO_PORTF_DATA_R = 0x00;   // LED is off
        }
      }
    }
  }
}



