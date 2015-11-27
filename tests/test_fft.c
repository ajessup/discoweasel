#include "fft.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <complex.h>

#define SAMPLE_LENGTH   1024
#define BASE            1500
#define FUDGE_1         0.4
#define FUDGE_2         2.0

void assert_f(double test, double expected, double epsilon);

int main(void) {
    // Compute an array that is a sine wave
    complex double wave[SAMPLE_LENGTH];
    double complex *transform;

    printf("Running tests for FFT: ");

    // Compute two superimposed sine waves
    for(int i=0;i<SAMPLE_LENGTH;i++){
        wave[i] = 
              (int)(sin(((double)i)*FUDGE_1) * BASE)
            + (int)(sin(((double)i)*FUDGE_2) * BASE)
            + BASE;
    }

    // Calc the FT
    transform = calc_fft(wave, SAMPLE_LENGTH);

    // Test a couple of values from random points in the returned array
    assert_f(creal(wave[491]),4450.000000,0.001);
    assert_f(fabs(creal(*(transform+491))),624.887115,0.001);

    assert_f(creal(wave[996]),2666.000000,0.001);
    assert_f(fabs(creal(*(transform+996))),2430.1315,0.001);

    printf(" OK!\n");

    free(transform);
}

/**
 * Assert two floating point values are the same, and exit gracefully
 * if they are not.
 */
void assert_f(double test, double expected, double epsilon){
    if(fabs(test - expected)>epsilon){
        printf("Fail\n");
        printf("  Expecting %f [+/- %f], got %f\n", test, epsilon, expected);
        exit(0);
    }else{
        printf(".");
    }
}

