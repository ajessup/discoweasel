#include "gpio.h"
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
    SYSCTL_RCGC2_R |= 0x00000020;     // 1) F clock @TODO This is a legacy register, should switch for the correct ones
    //delay = SYSCTL_RCGC2_R;           // delay   
    GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock PortF PF0  
    GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0       
    GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog function
    GPIO_PORTF_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL  
    GPIO_PORTF_DIR_R = 0x0E;          // 5) PF4,PF0 input, PF3,PF2,PF1 output   
    GPIO_PORTF_AFSEL_R = 0x00;        // 6) no alternate function
    GPIO_PORTF_PUR_R = 0x11;          // enable pullup resistors on PF4,PF0       
    GPIO_PORTF_DEN_R = 0x1F;          // 7) enable digital pins PF4-PF0        

    //Configure PF4 for interrupts
    GPIO_PORTF_IS_R &=  ~0x10;  // Set the IS bit to true, since we are detecting edges
    GPIO_PORTF_IBE_R &= ~0x10; // Set IBE bit to false, since we only want to trigger the falling edge, not both edges
    GPIO_PORTF_IEV_R &= ~0x10; // Set the IVE bit to false (to trigger on falling rather than rising edge)
    GPIO_PORTF_ICR_R = 0x10;   // Clear the ICR flag which will later be set when the interrupt is triggered
                             // Note: *Setting* this bit acts to *clear* the IM_R below
    GPIO_PORTF_IM_R |= 0x10;   // Set the IME bit to true (arm the interrupt)
    // We establish the priority of Port F interrupt to 5 by setting bits 23 – 21 in the NVIC_PRI7_R
    NVIC_PRI7_R = (NVIC_PRI7_R&0xFF00FFFF)|0x00A00000;
    // We activate Port F interrupts in the NVIC by setting bit 30 in the NVIC_EN0_R register
    NVIC_EN0_R = 0x40000000; // @TODO - Set this more cleanly
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
    ADC0_PC_R = 0x01;  // Peripheral control register. Set to sample at 125kHz (p 888

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

    currentRead.readCount = 0;
}

void ADC_Seq3_ISR(void) {
    ADC0_ISC_R = 0x08;     // Acknowledge and reset the interrupt
    if(currentRead.readCount < SAMPLE_LENGTH){
        currentRead.samples[currentRead.readCount] = ADC0_SSFIFO3_R;
        currentRead.readCount++;
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