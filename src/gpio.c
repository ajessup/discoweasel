#include <complex.h>
#include <stdbool.h>
#include "gpio.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "tm4c123gh6pm.h"


// @TODO This isn't the gpio file, it's the onboard buttons and switches file - should prob. refactor
// the GPIO init functions into their own file

// Subroutine to initialize port F pins for input and output
// PF4 and PF0 are input SW1 and SW2 respectively
// PF3,PF2,PF1 are outputs to the LED
// Inputs: None
// Outputs: None
// Notes: These five pins are connected to hardware on the LaunchPad
void PortF_Init(void){
    pf4_push_count = 1;
    //volatile unsigned long delay;
    GPIOIntRegister(GPIO_PORTF_BASE, GPIO_PortF_ISR);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);    //PORT B
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4 | GPIO_PIN_0);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1); // 5) PF4,PF0 input, PF3,PF2,PF1 output 
    GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_4 | GPIO_PIN_0, GPIO_FALLING_EDGE);
    GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_0 | GPIO_INT_PIN_4);
}

void GPIO_PortF_ISR(void) {
    GPIO_PORTF_ICR_R = 0x10; // Clear the flag, which will have been set by the CPU
    pf4_push_count = pf4_push_count + 1;
}

// We're gonna use Port E (PE3) for ADC0 sampling at 125kHz
void PortE_Init(void){
    SYSCTL_RCGCADC_R |= 0x01;      // Enable the ADC0 clock by setting bit 0 of SYSCTL_RGCCADC_R
    SYSCTL_RCGCGPIO_R |= 0x10;     // Enable port E clock pin
    GPIO_PORTE_DIR_R &= ~ 0x08;    // Make PE3 an input pin
    GPIO_PORTE_AFSEL_R |= 0x08;    // Enable the alternative function on PE3
    GPIO_PORTE_DEN_R &= ~0x08;     // Disable digital on PE3
    GPIO_PORTE_AMSEL_R |= 0x08;    // Enable analog on PE3

    // Now we set up the sampling timer, which runs at (1/bus speed)*prescale*period
    SYSCTL_RCGCTIMER_R |= 0x01;  // Activate timer0
    TIMER0_CTL_R = 0x00;         // Disable during setup
    TIMER0_CTL_R = 0x20;         // Enable timer0A trigger to ADC
    TIMER0_CFG_R = 0x00;         // 32 bit mode
    TIMER0_TAMR_R = 0x02;        // Periodic mode
    TIMER0_TAPR_R = 0;           // No prescale (ie. run at bus frequency)
    TIMER0_TAILR_R = 1000;       // Period
    TIMER0_IMR_R = 0x00;         // Disable interrupts
    TIMER0_CTL_R |= 0x01;        // Enable the timer

    // Now set up ADC0
    ADC0_PC_R = 0x01;  // Peripheral control register. Set to sample at 125kHz (p 888)

    // We're going to use sequencer 3 (the simplest one, only takes one sample)
    ADC0_ACTSS_R &= ~0x08; // ...first disable it by writing a 0 to bit 3 in ADC_ACTSS_R
    ADC0_SSPRI_R = 0x3210; // ...set sequencer 3 priority to lowest (honestly don't know why)
    ADC0_EMUX_R =
        (ADC0_EMUX_R&0xFFFF0FFF)+0x5000; // Set the ADC to be triggered by the timer
    ADC0_SSMUX3_R = 4;     // PE3 is analog channel 4 (@TODO not sure about this... does this point to AIN4?)
    ADC0_SSCTL3_R = 0x06;  // Set flag and end after the first sample
    ADC0_IM_R |= 0x08;     // Enable interrupts once the conversion is complete by setting bit 3
    ADC0_ACTSS_R |= 0x08;  // Enable the sequencer

    NVIC_PRI4_R =
        (NVIC_PRI4_R&0xFFFF00FF)|0x00004000; // Set the interrupt handler for ADC0 seq. 3 to priorty 2
    NVIC_EN0_R = 1<<17;    // Enable ADC0 seq 3 interrupts

    samples.readCount = 0;
    currentRead.readCount = 0;
}

void ADC_Seq3_ISR(void) {
    ADC0_ISC_R = 0x08;     // Acknowledge and reset the interrupt
    if(currentRead.readCount < SAMPLE_LENGTH){
        currentRead.samples[currentRead.readCount] = (int)ADC0_SSFIFO3_R;
        currentRead.readCount++;
    }else{
        // Get a rolling average of the values in currenRead and add to the samples array
        if(samples.readCount < SEQUENCE_LENGTH){
            unsigned long complex sum = 0;
            for(int i=0;i<SAMPLE_LENGTH;i++){
                sum += currentRead.samples[i];
            }
            float complex sample_average = sum/SAMPLE_LENGTH;
            samples.samples[samples.readCount] = sample_average;
            samples.readCount++;
            currentRead.readCount = 0;
        }
    }
}

// Color    LED(s) PortF
// dark     ---    0
// red      R--    0x02
// blue     --B    0x04
// green    -G-    0x08
// yellow   RG-    0x0A
// sky blue -GB    0x0C
// white    RGB    0x0E
// pink     R-B    0x06

// @TODO copied from driverlib/cpu.c, should properly reference that lib at some point
void EnableInterupts(void)
{
    __asm("    cpsie   i\n"
          "    bx      lr");
}

void DisableInterupts(void)
{
    __asm("    cpsid   i\n"
          "    bx      lr");
}

void WaitForInterrupt(void)
{
    __asm("    wfi\n"
          "    bx      lr");
}