

// Pre-processor Directives
// Constant declarations to access port registers using 
// symbolic names instead of addresses

// #define GPIO_PORTF_DATA_R       (*((volatile unsigned long *)0x400253FC))
// #define GPIO_PORTF_DIR_R        (*((volatile unsigned long *)0x40025400))
// #define GPIO_PORTF_AFSEL_R      (*((volatile unsigned long *)0x40025420))
// #define GPIO_PORTF_PUR_R        (*((volatile unsigned long *)0x40025510))
// #define GPIO_PORTF_DEN_R        (*((volatile unsigned long *)0x4002551C))
// #define GPIO_PORTF_LOCK_R       (*((volatile unsigned long *)0x40025520))
// #define GPIO_PORTF_CR_R         (*((volatile unsigned long *)0x40025524))
// #define GPIO_PORTF_AMSEL_R      (*((volatile unsigned long *)0x40025528))
// #define GPIO_PORTF_PCTL_R       (*((volatile unsigned long *)0x4002552C))
// #define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))

#define SAMPLE_LENGTH 80

//   Function Prototypes

void PortF_Init(void);
void GPIO_PortF_ISR(void);

void PortE_Init(void);
void ADC_Seq3_ISR(void);

struct Sample{
	unsigned long samples[SAMPLE_LENGTH];
	unsigned short readCount;
};
struct Sample currentRead;

unsigned long pf4_push_count;

void EnableInterupts(void);
void DisableInterupts(void);
void WaitForInterrupt(void);