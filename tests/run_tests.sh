set -e
set -o pipefail # Fail if any process fails

echo "== Testing dynamic memory allocation lib =="
gcc -g -I../src ../src/heap.c HeapTestMain.c -o test_heap
./test_heap 

echo "== Testing Fast Fourier Transform Lib =="
gcc -g -I../src ../src/heap.c ../src/fft.c test_fft.c -o test_ffc
./test_ffc