#include <complex.h>

double complex * calc_fft(double complex input[], int length);
double complex * _fft(double complex input[], double complex twiddles[], int length, int origlength);
double complex * dft(double complex input[], int length);
