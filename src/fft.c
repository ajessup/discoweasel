/**
 * fft.c
 * @author "Andrew Jessup" <ajessup@gmail.com>
 *
 * A C implementation of the radix 2 cooley/tukey fft algorithm, adapted from
 * the python implementation presented by jaime.frio AT gmail.com at https://goo.gl/mcN8Bv
 **/ 

#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <stdio.h>
#include "fft.h"
#include "heap.h"

#ifndef M_PI
 #define M_PI   3.14159265358979323846264338327950288
#endif

/**
 * Perform the fast fourier transform on a signal input
 *
 * parameters:
 *   input, raw signal values, encoded in the real portion of absolute numbers
 *   length, the number of elements in the input array
 * returns:
 *   an array of size `length` transformed from `input` OR NULL if OOM
 */
double complex * calc_fft(double complex input[], int length) {
    double complex *twiddles = (double complex *)Heap_Calloc(length / 2 * sizeof(double complex));
    if(twiddles==NULL){
        return NULL;
    }
    for(int i=0;i<length/2;i++){
        double complex J = 2.0 * I;
        *(twiddles + i) =
            cexp(((double complex)M_PI * (double complex)i * J) / (double complex)length);
    }
    return _fft(input, twiddles, length, length);
    Heap_Free(twiddles);
}

/**
 * parameters:
 *   input, raw signal values, encoded in the real portion of absolute numbers
 *   twiddles, set of twiddle factors
 *   length, length of the original array
 *   origlength, length used to comptue the twiddle factors
 */
double complex* _fft(double complex input[], double complex twiddles[], int length, int origlength) {
    double complex *result = (double complex*)Heap_Calloc(length * sizeof(double complex));

    if(length % 2){
        return dft(input, length);
    }

    int halflength = floor(length / 2);

    double complex *input_even = (double complex*)Heap_Calloc(halflength * sizeof(double complex));
    double complex *input_odd = (double complex*)Heap_Calloc(halflength * sizeof(double complex));

    for(int i=0;i<length;i++) {
        if(i % 2){
            *(input_odd + (i/2)) = input[i];
        }else{
            *(input_even + (i/2)) = input[i];
        }
    }

    double complex *output_even = _fft(input_even, twiddles, halflength, origlength);
    double complex *output_odd = _fft(input_odd, twiddles, halflength, origlength);

    for(int k=0;k<halflength;k++){
        *(result+k) = *(result+k) +
            ( *(output_even + k) + *(output_odd + k) * twiddles[k * origlength / length] );
    }

    for(int k=halflength;k<length;k++){
        *(result+k) = *(result+k) +
            ( *(output_even + (k-halflength)) - *(output_odd + (k-halflength)) * twiddles[(k-halflength) * origlength / length] );
    }

    Heap_Free(input_even);
    Heap_Free(input_odd);
    Heap_Free(output_even);
    Heap_Free(output_odd);

    return result;
}

/**
 * Compute the discrete fourier transform for a signal
 * 
 * parameters:
 *   input, raw signal values, encoded in the real portion of absolute numbers
 *   length, the number of elements in the input array
 * returns:
 *   an array of size `length` transformed from `input` 
 */
double complex* dft(double complex input[], int length) {
    double complex *result = (double complex *)Heap_Calloc(length * sizeof(double complex));

    if(length == 1){
        *(result) = input[0];
    }else if(length == 2){
        *(result) = (input[0] + input[1]);
        *(result+1) = (input[0] - input[1]);
    }else if(length == 4){
        double complex a = (input[0] + input[2]);
        double complex b = (input[0] - input[2]);
        double complex c = (input[1] + input[3]);
        double complex d = (input[1] - input[3]);

        *(result) = a + c;
        *(result+1) = b + (1.0 * I * d);
        *(result+2) = a - c;
        *(result+3) = b - (1.0 * I * d);
    }else{
        double complex *twiddles = (double complex *)Heap_Calloc(length * sizeof(double complex));
        // Calculate the first half of the twiddle factors
        for(int k=0;k<(floor(length/2)+1);k++){
        //     twiddles = [math.e**(inv*2j*math.pi*k/N) for k in xrange(M)]+[N]
            double complex J = 2.0 * I;
            *(twiddles + k) =
                cexp(((double complex)M_PI * (double complex)k * J) / (double complex)length);
        }
        // Second hald of the twiddle factors are the conjugates of the first half
        for(int k=(floor(length/2)+1);k<length;k++){
            *(twiddles + k) = 
                *(twiddles + k)
                + conj(*(twiddles + (length-k)));
        }
        // Do the transform...
        for(int k=0;k<length;k++){
            for(int n=0;n<length;n++){
                *(result+k) 
                    = *(result+k) 
                      + (input[n] * *(twiddles + (n * k % length)));
            }
        }
    }
    return result;
}


