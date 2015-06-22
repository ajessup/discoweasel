/**
 * This code uses ports PA2-7 on the TM4C123 to implement SSI to drive a Nokia 5110 display
 * Design notes:
 *   https://docs.google.com/document/d/1t_GjTHUdOiABjLDzbjnnqfJqadzx_u5LQwFda0S2yi4/edit
 *
 * This code assumes the following pinout:
 *  PA7 (pin 10) to RST (Reset) on Nokia - required on startup to reset the board
 *  PA6 (pin 9) to  DC (Data Command) - this is high? when sending commands, low when sending data
 *  PA5 SSI0Tx (pin 22) to  Din (Data In) or MOSI - this is the pin we will send data on
 *  PA4 SSI0Rx (pin 13) to nothing (since we are not recieving data from the display)
 *  PA3 SSI0Fss (pin 12) to CE (Chip Enable) - when this is high, the display will listen
 *  PA2 SSI0Clk (pin 11) to Clk (Clock) - this provides a timing pulse to synchronise every command
 *  The backlight is not pinned in, later we'll hook this up to PA4
 * 
 * A few things to know when using SPI to connect to this device:
 *  * The data bit in transmission is read on the rising edge of the clock
 *  * We are using the Freescale SPI data format as described in 15.3.4.2 of the Tiva C data sheet
 **/

#include "nokia.h"

#define SSI_STATUS_BUSY      0x10
#define SSI_RX_FIFO_FULL     0x08
#define SSI_RX_FIFO_NEMPTY   0x04
#define SSI_TX_FIFO_NFULL    0x02
#define SSI_TX_FIFO_EMPTY    0x01

#define SSI0_CR0_R           (*((volatile unsigned long *)0x40008000)) // Register 1 - SSI0 Control Register 0
#define SSI0_CR1_R           (*((volatile unsigned long *)0x40008004)) // Register 2 - SSI0 Control Register 1
#define SSI0_DATA_R          (*((volatile unsigned long *)0x40008008)) // Register 3 - SSI0 Data Register
#define SSI0_STATUS_R        (*((volatile unsigned long *)0x4000800C)) // Register 4 - SSI0 Status register
#define SSI0_CCR_R           (*((volatile unsigned long *)0x40008FC8)) // Register 11 - SS0 Clock Control Register
#define SSI0_SSICPS_R        (*((volatile unsigned long *)0x40008010)) // Register 5 - SSI Clock Prescale

#define GPIO_PORTA_4_ENABLE  0x08
#define GPIO_PORTA_6_ENABLE  0x40
#define GPIO_PORTA_7_ENABLE  0x80

#define GPIO_PORTA_DATA_R    (*((volatile unsigned long *)0x400043FC)) // Regsiter 1 - GPIO Data register (+full bitmask)
#define GPIO_PORTA_DIR_R     (*((volatile unsigned long *)0x40004400)) // Register 2 - GPIO Direction Control Register
#define GPIO_PORTA_AFSEL_R   (*((volatile unsigned long *)0x40004420)) // Register 10 - GPIO AFSEL for Port A
#define GPIO_PORTA_DEN_R     (*((volatile unsigned long *)0x4000451C)) // Register 18 - GPIO Port A digital enable
#define GPIO_PORTA_GPIOCTL_R (*((volatile unsigned long *)0x4000452C)) // Register 22 - GPIOCTL selector for port A

#define SYSCTL_RCGCGPIO_R    (*((volatile unsigned long *)0x400FE608)) // Register 60 - GPIO RMCGC
#define SYSCTL_RCGSSI_R      (*((volatile unsigned long *)0x400FE61C)) // Register 64 - SSI RMCGC
#define SYSCTL_RCGC1_R	     (*((volatile unsigned long *)0x400FE104)) // Register 135 Run mode clock gating control register

/**
 * To initilaise SSI on the Tiva C, we perform the following as described in section 15.4 of the Tiva C data sheet
 */

void TM4C123_SSI_Init() {	
	// Enable the SSI module using the RCGCSSI register
	// The RCGCSSI register provides software the capability to enable and disable the SSI modules in
	// Run mode. 
	
	// When enabled, a module is provided a clock and accesses to module registers are
	// allowed. When disabled, the clock is disabled to save power and accesses to module registers
	// generate a bus fault. This register provides the same capability as the legacy Run Mode Clock
	// Gating Control Register n RCGCn registers specifically for the watchdog modules and has the
	// same bit polarity as the corresponding RCGCn bits.
	
	SYSCTL_RCGSSI_R   |= 0x01;
	
	// Enable the clock for PA via the RCGCGPIO register
	
	SYSCTL_RCGCGPIO_R |= 0x01;
	
	// Set the GPIO AFSEL bits for the appropriate pins, we want to select the alternative function
	// for the SSI pins (PA2, PA3 and PA5) and make sure it's disabled for PA4, PA6 and PA7
	
	GPIO_PORTA_AFSEL_R = 0x2C;
	
	// Set the direction register for PA4, PA6 and PA7 to output
	GPIO_PORTA_DIR_R |= 0xD0; 
	
	// Enable digital I/O on PA2,3,4,5,6,7
	GPIO_PORTA_DEN_R |= 0xFE;
	
	// Configure the PMCn fileds in the GPIOCTL register to assign the SSI signals to the appropriate pins
	
	// Set PA2, PA3 and PA5 as SSI (function 2)
	GPIO_PORTA_GPIOCTL_R = (GPIO_PORTA_GPIOCTL_R & 0xFF0F00FF) + 0x00202200;
	
	// Set PA4, PA6 and PA7 as GPIO (function 0)
	GPIO_PORTA_GPIOCTL_R = (GPIO_PORTA_GPIOCTL_R & 0x00F0FFFF);
	
	// Having enabled SSI0, we now need to configure it...
	
	// Clear the SEE bit in the SSICR1 register
	SSI0_CR1_R &= 0x1D;
	
	// Select SSI master mode (set SSICR1 to 0x00000000)
	SSI0_CR1_R = 0x00000000; // @TODO - This could be more elegant
	
	// Configure the SSI clock source to the whatever the system clock is, and not the software controled precision oscillator
	SSI0_CCR_R = 0x0;
	
	// Configure the clock prescale divisor by writing the SSICPRS register
	// The final bit rate will be = sysclock speed / (PsD * (1 + SCR)) (see below for SCR details)
	// We assume the interal clock is set to 50MHz, and divide by 16 for a bus speed of 3.125MHz
	SSI0_SSICPS_R = 0x10;
	
	// Write the SSICR0 register with:
	
	//  - the Serial Clock Rate (SCR) (we will set to 0)
	SSI0_CR0_R = (SSI0_CR0_R & 0x00000FFF) +
	//  - the clock phase (SPH) (set to 0x0, as we wish to capture data on the second clock edge transition)
		(1<<7) +
	//  - and polarity (SPO) (set to 0x0, as the clock should be LOW when no data is being transfered)
		(1<<6) +
	//  - the protocol (FRF) mode to be Freescale SPI (0x0)
		(0<<4) +
	//  - the data size (DSS) (8 bits)
		0x7;
	
	// Finally, now we're done configuring, set the SEE bit in the SSICR1 register
	SSI0_CR1_R |= 0x02;
}

/**
 * To send a single command to the display, I need to do the following:
 *  * Set the DC line low
 *  * Set the CE line low (the SSI module will do this)
 *  * Send a byte down the wire, most significant bit first (the SSI module will do this)
 *  * Set the CE line high again (the SSI module will do this)
 */ 

void Nokia_WriteCmd(char cmd) {
	// In sending a command, wait until the SSI0 FIFO buffer is free
	while(SSI0_STATUS_R & SSI_STATUS_BUSY){};
	// Set the DC line (PA 6) to low
	GPIO_PORTA_DATA_R &= ~GPIO_PORTA_7_ENABLE;
	// Write the data
	SSI0_DATA_R = cmd;
	// Wait again until the buffer is free
	while(SSI0_STATUS_R & SSI_STATUS_BUSY){};
}

/**
 * To initialise the display, we need to perform the following:
 *  * Put the RESET pin (PA7) LOW then HIGH
 *  * Send a series of commands to the display
 */

void Nokia_InitDisplay() {
	TM4C123_SSI_Init();
	
	GPIO_PORTA_DATA_R = 0x00;
	for(unsigned long delay=0; delay<10; delay=delay+1); // @TODO - Need a smarter way to do this
	GPIO_PORTA_DATA_R = GPIO_PORTA_7_ENABLE;
	
	Nokia_WriteCmd(0x21); // Enable LCD extended commands
	Nokia_WriteCmd(0xB8); // Set Vop (contrast)
    Nokia_WriteCmd(0x04); // Set temp coefficient
	Nokia_WriteCmd(0x14); // LCD Bias Mode 1:40
	Nokia_WriteCmd(0x20); // Back to the basic command set
	Nokia_WriteCmd(0x09); // LCD all segments up
}