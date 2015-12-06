#include <complex.h>

#define SAMPLE_LENGTH   4 // Number of samples to take and average
#define SEQUENCE_LENGTH 128 // Number of averages samples to collect to analzye

void PortF_Init(void);
void GPIO_PortF_ISR(void);

void PortE_Init(void);
void ADC_Seq3_ISR(void);

struct Sample{
	int samples[SAMPLE_LENGTH];
	unsigned short readCount;
};
struct Sample currentRead;

struct Sequence{
    float complex samples[SEQUENCE_LENGTH];
    unsigned short readCount;
};
struct Sequence samples;

extern unsigned long pf4_push_count;

void EnableInterupts(void);
void DisableInterupts(void);
void WaitForInterrupt(void);