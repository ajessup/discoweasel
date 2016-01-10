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

#define PART_TM4C123GH6PM   1

#include <stdint.h>
#include <stdbool.h>

#include "nokia.h"
#include "inc/hw_memmap.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"

// @TODO - Refactor all of these to use defs from #include "tm4c123gh6pm.h"

#define SSI_STATUS_BUSY      0x10
#define SSI_RX_FIFO_FULL     0x08
#define SSI_RX_FIFO_NEMPTY   0x04
#define SSI_TX_FIFO_NFULL    0x02
#define SSI_TX_FIFO_EMPTY    0x01

#define SSI0_STATUS_R        (*((volatile unsigned long *)0x4000800C)) // Register 4 - SSI0 Status register

#define GPIO_PORTA_DATA_R    (*((volatile unsigned long *)0x400043FC)) // Regsiter 1 - GPIO Data register (+full bitmask)
#define GPIO_PORTA_DIR_R     (*((volatile unsigned long *)0x40004400)) // Register 2 - GPIO Direction Control Register
#define GPIO_PORTA_AFSEL_R   (*((volatile unsigned long *)0x40004420)) // Register 10 - GPIO AFSEL for Port A
#define GPIO_PORTA_DEN_R     (*((volatile unsigned long *)0x4000451C)) // Register 18 - GPIO Port A digital enable

#define GPIO_PORTA_6_ENABLE  0x40
#define GPIO_PORTA_7_ENABLE  0x80

/**
 * To initilaise SSI on the Tiva C, we perform the following as described in section 15.4 of the Tiva C data sheet
 */

void TM4C123_SSI_Init() {   
    
    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    
    GPIOPinConfigure(GPIO_PA2_SSI0CLK);
    GPIOPinConfigure(GPIO_PA3_SSI0FSS);
    GPIOPinConfigure(GPIO_PA4_SSI0RX);
    GPIOPinConfigure(GPIO_PA5_SSI0TX);

    GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5);

    GPIO_PORTA_AFSEL_R = 0x2C;
    
    // Set the direction register for PA4, PA6 and PA7 to output
    GPIO_PORTA_DIR_R |= 0xC0; // @TODO - Make PA4 out as well, set to |= 0xD0
    
    // Enable digital I/O on PA2,3,4,5,6,7
    GPIO_PORTA_DEN_R |= 0xEC;  // @TODO - Set to xFC to enable PA4
    
    //Having enabled SSI0, we now need to configure it...
    SSIConfigSetExpClk(
        SSI0_BASE, 
        SysCtlClockGet(), 
        SSI_FRF_MOTO_MODE_0, 
        SSI_MODE_MASTER, 
        200000, 
        8
    );

    SSIEnable(SSI0_BASE);
}

/**
 * To send a single command to the display, I need to do the following:
 *  * Set the DC line low
 *  * Set the CE line low (the SSI module will do this)
 *  * Send a byte down the wire, most significant bit first (the SSI module will do this)
 *  * Set the CE line high again (the SSI module will do this)
 */ 

void Nokia_Write(char data, bool isCmd) {
    // In sending a command, wait until the SSI0 FIFO buffer is free
    while(SSI0_STATUS_R & SSI_STATUS_BUSY){};
    if(isCmd){
        // Set the DC line (PA 6) to low
        GPIO_PORTA_DATA_R &= ~GPIO_PORTA_6_ENABLE;
    }else{
        // Set the DC line (PA 6) to low
        GPIO_PORTA_DATA_R |= GPIO_PORTA_6_ENABLE;
    }
    
    // Write the data
    SSIDataPut(SSI0_BASE, data);

    // Wait again until the buffer is free
    while(SSI0_STATUS_R & SSI_STATUS_BUSY){};
}

unsigned char Reverse_bits(unsigned char num){

    int i=7; //size of unsigned char -1, on most machine is 8bits
    unsigned char j=0;
    unsigned char temp=0;

    while(i>=0){
      temp |= ((num>>j)&1)<< i;
      i--;
      j++;
    }
    return(temp); 
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
    
    Nokia_Write(0x21, true); // Enable LCD extended commands
    Nokia_Write(0xC0, true); // Set Vop (contrast)
    Nokia_Write(0x04, true); // Set temp coefficient
    Nokia_Write(0x14, true); // LCD Bias Mode 1:40
    Nokia_Write(0x20, true); // Back to the basic command set
    Nokia_Write(0x0c, true);
}

void Nokia_ClearScreen(void) {
    for(unsigned short row=0; row<6; row=row+1){
        for(unsigned short col=0; col<84; col=col+1){
            Nokia_Write(0x00, false);
        }
    }
}

void Nokia_WriteImg(bool img[NOKIA_SCREEN_COLS][NOKIA_SCREEN_ROWS]) {
    // Write the image
    for(short cur_row_segment=NOKIA_SCREEN_V_SEGMENTS-1;cur_row_segment>=0;cur_row_segment--){
        for(short cur_col=0;cur_col<NOKIA_SCREEN_COLS;cur_col++){
            char segment_val =
                (1 * (img[cur_col][cur_row_segment*NOKIA_SCREEN_V_SEGMENTS_HEIGHT + 7 ])) +
                (1 * (img[cur_col][cur_row_segment*NOKIA_SCREEN_V_SEGMENTS_HEIGHT + 6 ])<< 1) +
                (1 * (img[cur_col][cur_row_segment*NOKIA_SCREEN_V_SEGMENTS_HEIGHT + 5 ])<< 2) +
                (1 * (img[cur_col][cur_row_segment*NOKIA_SCREEN_V_SEGMENTS_HEIGHT + 4 ])<< 3) +
                (1 * (img[cur_col][cur_row_segment*NOKIA_SCREEN_V_SEGMENTS_HEIGHT + 3 ])<< 4) +
                (1 * (img[cur_col][cur_row_segment*NOKIA_SCREEN_V_SEGMENTS_HEIGHT + 2 ])<< 5) +
                (1 * (img[cur_col][cur_row_segment*NOKIA_SCREEN_V_SEGMENTS_HEIGHT + 1 ])<< 6) +
                (1 * (img[cur_col][cur_row_segment*NOKIA_SCREEN_V_SEGMENTS_HEIGHT + 0 ])<< 7);
            Nokia_Write(segment_val, false);
        }
    }
}
