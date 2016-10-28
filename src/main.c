// LaunchPad built-in hardware
// SW1 left switch is negative logic PF4 on the Launchpad
// SW2 right switch is negative logic PF0 on the Launchpad
// red LED connected to PF1 on the Launchpad
// blue LED connected to PF2 on the Launchpad
// green LED connected to PF3 on the Launchpad

#include <complex.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "inc/hw_memmap.h"

#include "heap.h"
#include "gpio.h"
#include "tm4c123gh6pm.h"
#include "nokia.h"
#include "fft.h"
#include "lpd8806_ledstrip.h"

#define NUM_BUCKETS 16 // Number of buckets (one bucket per LED)
#define BUCKET_SIZE  5  // Bucket size in terms of samples

// 2. Declarations Section
//   Global Variables
unsigned long SW1, SW1_prev,SW2;  // input from PF4,PF0
unsigned long Out;      // outputs to PF3,PF2,PF1 (multicolor LED)

unsigned long pf4_push_count;

void delay100ms (unsigned  long time  )
{
    while  (time>0)
    {
        int i = 1333333;   // This number is equivalent to 100 ms
        while (i>0)
        {
            i--;
        }
        time--;
    }
}

// Allocate a frame buffer for a 32 pixel strip
#define NUM_PIXELS 32
LPD8806_LedStrip_Handle_t hLed;
uint8_t frameBuffer[LPD8806_LEDSTRIP_FRAMEBUF_SIZE(NUM_PIXELS)];

void Lights_Init(void){

  SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI1);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

  GPIOPinConfigure(GPIO_PF2_SSI1CLK);
  GPIOPinConfigure(GPIO_PF1_SSI1TX);

  GPIOPinTypeSSI(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 );

  hLed = LPD8806_LedStrip_Init(SysCtlClockGet(), 1, 200000);

  if(!hLed)
  {
      // handle error
  }
  // Assign the frame buffer
  LPD8806_LedStrip_SetFrameBuffer(hLed, frameBuffer, NUM_PIXELS);
}

// 3. Subroutines Section
// MAIN: Mandatory for a C Program to be executable
int main(void){

  DisableInterupts();

  //SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

  Lights_Init();

  LPD8806_LedStrip_Clear(hLed);
  LPD8806_LedStrip_Update(hLed);

  //unsigned short mode=0;
  bool screenbuffer[NOKIA_SCREEN_COLS][NOKIA_SCREEN_ROWS];

  //TExaS_Init(SW_PIN_PF40,LED_PIN_PF321);
  PortF_Init();        // Call initialization of port PF4, PF3, PF2, PF1, PF0
  PortE_Init();        // Init ADC and PE3

  Nokia_InitDisplay();

  Heap_Init();

  // Enable interrupts
  EnableInterupts();

  Nokia_ClearScreen();


  while(1){
    DisableInterupts();
    short color = pf4_push_count % 8;
    switch(color){
        case 0:
            GPIO_PORTF_DATA_R = 0x02;
            break;
        case 1:
            GPIO_PORTF_DATA_R = 0x04;
            break;
        case 2:
            GPIO_PORTF_DATA_R = 0x08;
            break;
        case 3:
            GPIO_PORTF_DATA_R = 0x0A;
            break;
        case 4:
            GPIO_PORTF_DATA_R = 0x0C;
            break;
        case 5:
            GPIO_PORTF_DATA_R = 0x0E;
            break;
        case 6:
            GPIO_PORTF_DATA_R = 0x06;
            break;
        case 7:
        default:
            GPIO_PORTF_DATA_R = 0x00;
            break;
    }

    // Read in audio samples, and if we have something write it to the screen
    if(samples.readCount == SEQUENCE_LENGTH) {
        float complex * transform;
        transform = calc_fft(samples.samples, SEQUENCE_LENGTH);

        // Zero out the first response from transform since it collects
        // a lot of noise
        transform[0] = (complex double)0;

        // Max read of ADC is 12bits (4024), bit-shift 7 to max of 36
        for(short current_bucket=0;current_bucket<NUM_BUCKETS;current_bucket++){
            unsigned long bucket_total = 0;
            for(short bucket_pos=0;bucket_pos<BUCKET_SIZE;bucket_pos++){
                short col_pos = (current_bucket*BUCKET_SIZE)+bucket_pos;
                unsigned long sample = abs(creal(transform[col_pos]));
                short scaledSample = sample >> 7;
                bucket_total += sample;

                // Draw the column
                for(short j=0;j<NOKIA_SCREEN_ROWS;j++){
                   if(scaledSample>j){
                     screenbuffer[col_pos][j] = true;
                   }else{
                     screenbuffer[col_pos][j] = false;
                   }
                }

                // Draw the bucket boundary
                if(bucket_pos==0){
                   for(short j=0;j<NOKIA_SCREEN_ROWS;j++){
                     screenbuffer[col_pos][j] = screenbuffer[col_pos][j] || (j % 2);
                   }
                }
            }
            unsigned long bucket_average = floor(bucket_total / BUCKET_SIZE);
            short scaled_bucket_average = bucket_average >> 7;
            for(short j=(BUCKET_SIZE*current_bucket);j<(BUCKET_SIZE*(current_bucket+1));j++){
               screenbuffer[j][scaled_bucket_average] = true;
            }

        }

        samples.readCount = 0;
        Nokia_WriteImg(screenbuffer);
        Heap_Free(transform);
    }
    EnableInterupts();
    WaitForInterrupt();
  }
}


int logicalRightShift(int x, int n) {
    return (unsigned)x >> n;
}
