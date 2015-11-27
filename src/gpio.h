#include <complex.h>

#define SAMPLE_LENGTH 1024

void PortF_Init(void);
void GPIO_PortF_ISR(void);

void PortE_Init(void);
void ADC_Seq3_ISR(void);

struct Sample{
	double complex samples[SAMPLE_LENGTH];
	unsigned short readCount;
};
struct Sample currentRead;

extern unsigned long pf4_push_count;

void EnableInterupts(void);
void DisableInterupts(void);
void WaitForInterrupt(void);