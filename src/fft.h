#include <complex.h>

float complex * calc_fft(float complex input[], int length);
float complex * _fft(float complex input[], float complex twiddles[], int length, int origlength);
float complex * dft(float complex input[], int length);
