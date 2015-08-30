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
  SYSCTL_RCGC2_R |= 0x00000020;     // 1) F clock
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

    //
    // Read PRIMASK and enable interrupts.
    //
    // __asm(
    //       "    mrs     r0, PRIMASK\n"
    //       "    cpsie   i\n"
    //       "    bx      lr"
    //       : "=r" (ui32Ret));
	
    __asm("    cpsie   i\n"
          "    bx      lr");


    //
    // The return is handled in the inline assembly, but the compiler will
    // still complain if there is not an explicit return here (despite the fact
    // that this does not result in any code being produced because of the
    // naked attribute).
    //
}

uint32_t DisableInterupts(void)
{
    uint32_t ui32Ret;

    //
    // Read PRIMASK and disable interrupts.
    //
    __asm("    mrs     r0, PRIMASK\n"
          "    cpsid   i\n"
          "    bx      lr\n"
          : "=r" (ui32Ret));

    //
    // The return is handled in the inline assembly, but the compiler will
    // still complain if there is not an explicit return here (despite the fact
    // that this does not result in any code being produced because of the
    // naked attribute).
    //
    return(ui32Ret);
}

void WaitForInterrupt(void)
{
    //
    // Wait for the next interrupt.
    //
    __asm("    wfi\n"
          "    bx      lr\n");
}