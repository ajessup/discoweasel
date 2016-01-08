#include <complex.h>
#include <stdbool.h>
#include "gpio.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#include "driverlib/timer.h"
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
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0); 
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE); 
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0); 

    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3); // Make PE3 the input for the ADC (PE3 alternate is AIN0)

    // Now we set up the sampling timer, which runs at (1/bus speed)*prescale*period
    TimerDisable(TIMER0_BASE, 0);
    TimerControlTrigger(TIMER0_BASE, TIMER_A, true); // Set timer to trigger the ADC
    TimerConfigure(TIMER0_BASE, TIMER_CFG_A_PERIODIC); // Periodic mode         
    TimerLoadSet(TIMER0_BASE, TIMER_A, 1000); // Set Period
    TimerPrescaleSet(TIMER0_BASE, TIMER_A, 0); // No prescale (ie. run at bus frequency)
    TimerIntDisable(TIMER0_BASE, 0xFFFFFFFF); // Disable interrupts for Timer A
    TimerEnable(TIMER0_BASE, TIMER_A);

    // Now set up ADC0
    ADCClockConfigSet(ADC0_BASE, ADC_CLOCK_SRC_PIOSC | ADC_CLOCK_RATE_EIGHTH, 16);
    ADCSequenceDisable(ADC0_BASE, 3);
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_TIMER, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_IE | ADC_CTL_END | ADC_CTL_CH4);
    ADCSequenceEnable(ADC0_BASE, 3);

    NVIC_PRI4_R =
        (NVIC_PRI4_R&0xFFFF00FF)|0x00004000; // Set the interrupt handler for ADC0 seq. 3 to priorty 2
    NVIC_EN0_R = 1<<17;    // Enable ADC0 seq 3 interrupts

    samples.readCount = 0;
    currentRead.readCount = 0;

    ADCIntEnable(ADC0_BASE, 3);
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