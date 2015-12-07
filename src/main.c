// LaunchPad built-in hardware
// SW1 left switch is negative logic PF4 on the Launchpad
// SW2 right switch is negative logic PF0 on the Launchpad
// red LED connected to PF1 on the Launchpad
// blue LED connected to PF2 on the Launchpad
// green LED connected to PF3 on the Launchpad

#include <complex.h>
#include <math.h>
#include <stdlib.h>

#include "heap.h"
#include "gpio.h"
#include "tm4c123gh6pm.h"
#include "nokia.h"
#include "scrooge.h"
#include "fft.h"

#define NUM_BUCKETS 10 // Number of buckets (one bucket per LED)
#define BUCKET_SIZE  8  // Bucket size in terms of samples

// 2. Declarations Section
//   Global Variables
unsigned long SW1, SW1_prev,SW2;  // input from PF4,PF0
unsigned long Out;      // outputs to PF3,PF2,PF1 (multicolor LED)

unsigned long pf4_push_count;

// 3. Subroutines Section
// MAIN: Mandatory for a C Program to be executable
int main(void){
  DisableInterupts();
  //unsigned short mode=0;
  char screenbuffer[84][6];

  //TExaS_Init(SW_PIN_PF40,LED_PIN_PF321); 
  PortF_Init();        // Call initialization of port PF4, PF3, PF2, PF1, PF0    
  PortE_Init();        // Init ADC and PE3
    
  Nokia_InitDisplay();

  Heap_Init();

  // Show scrooge while loading...
  Nokia_WriteImg(scroogeImg);

  // Enable interrupts
  EnableInterupts();

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

        // Max read of ADC is 12bits (4024), bit-shift 7 to max of 36
        for(short current_bucket=0;current_bucket<NUM_BUCKETS;current_bucket++){
            for(short bucket_pos=0;bucket_pos<BUCKET_SIZE;bucket_pos++){
                short col_pos = (current_bucket*BUCKET_SIZE)+bucket_pos;
                long scaledSample = (abs(creal(transform[col_pos])) >> 7);
                for(short j=0;j<NOKIA_SCREEN_V_SEGMENTS;j++){
                    long segmentVal = scaledSample - (j*NOKIA_SCREEN_V_SEGMENTS_HEIGHT);
                    if(segmentVal < 0){
                        screenbuffer[col_pos][5-j] = 0x00;
                    }else if(segmentVal > NOKIA_SCREEN_V_SEGMENTS_HEIGHT){
                        screenbuffer[col_pos][5-j] = 0xFF;
                    }else{
                        screenbuffer[col_pos][5-j] = ((char)1 << segmentVal)-1;
                    }
                }                
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
