#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include "fft.h"
#include "heap.h"

#define SAMPLE_LENGTH   128
#define BASE            1500
#define FUDGE_1         0.4
#define FUDGE_2         2.0

void assert_f(float test, float expected, float epsilon);

int main(void) {
    // Compute an array that is a sine wave
    complex float wave[SAMPLE_LENGTH];
    float complex *transform;

    Heap_Init();

    printf("Running tests for FFT: ");

    // Compute two superimposed sine waves
    for(int i=0;i<SAMPLE_LENGTH;i++){
        wave[i] = 
              (int)(sin(((float)i)*FUDGE_1) * BASE)
            + (int)(sin(((float)i)*FUDGE_2) * BASE)
            + BASE;
    }

    // Calc the FT
    transform = calc_fft(wave, SAMPLE_LENGTH);
    if(transform == NULL){
        printf("\nFAIL: Error calculating FFT (Out of Memory?) for sample legnth %d\n", SAMPLE_LENGTH);
        exit(1);
    }

    // Test a couple of values from random points in the returned array
    assert_f(creal(wave[33]),2349.000000,0.001);
    assert_f(fabs(creal(*(transform+33))),1956.0641,0.001);

    assert_f(creal(wave[120]),1766.000000,0.001);
    assert_f(fabs(creal(*(transform+120))),42704.632812,0.001);

    printf(" OK!\n");

    Heap_Free(transform);
}

/**
 * Assert two floating point values are the same, and exit gracefully
 * if they are not.
 */
void assert_f(float test, float expected, float epsilon){
    if(fabs(test - expected)>epsilon){
        printf("Fail\n");
        printf("  Expecting %f [+/- %f], got %f\n", expected, epsilon, test);
        exit(0);
    }else{
        printf(".");
    }
}

