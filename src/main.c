// LaunchPad built-in hardware
// SW1 left switch is negative logic PF4 on the Launchpad
// SW2 right switch is negative logic PF0 on the Launchpad
// red LED connected to PF1 on the Launchpad
// blue LED connected to PF2 on the Launchpad
// green LED connected to PF3 on the Launchpad

#include "gpio.h"
#include "tm4c123gh6pm.h"
#include "nokia.h"
#include "scrooge.h"
#include <math.h>

// 2. Declarations Section
//   Global Variables
unsigned long SW1, SW1_prev,SW2;  // input from PF4,PF0
unsigned long Out;      // outputs to PF3,PF2,PF1 (multicolor LED)

unsigned long pf4_push_count;

// 3. Subroutines Section
// MAIN: Mandatory for a C Program to be executable
int main(void){
  //unsigned short mode=0;
  char screenbuffer[84][6];

  //TExaS_Init(SW_PIN_PF40,LED_PIN_PF321); 
  PortF_Init();        // Call initialization of port PF4, PF3, PF2, PF1, PF0    
  PortE_Init();        // Init ADC and PE3
    
  Nokia_InitDisplay();

  // Show scrooge while loading...
  //DisableInterupts();
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
        if(currentRead.readCount == SAMPLE_LENGTH) {
            // Max read of ADC is 12bits (4024), bit-shift 7 to max of 36
            for(int i=0;i<80;i++){
				long scaledSample = (currentRead.samples[i] >> 7);
				for(short j=0;j<6;j++){
					long segmentVal = scaledSample - (j*8);
					if(segmentVal < 0){
						screenbuffer[i][5-j] = 0x00;
					}else if(segmentVal > 8){
						screenbuffer[i][5-j] = 0xFF;
					}else{
						screenbuffer[i][5-j] = ((char)1 << segmentVal)-1;
					}
				}
            }
            currentRead.readCount = 0;
            //Nokia_ClearScreen();
            Nokia_WriteImg(screenbuffer);
        }
        EnableInterupts();
        WaitForInterrupt();
    }
}


int logicalRightShift(int x, int n) {
    return (unsigned)x >> n;
}
