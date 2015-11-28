set -e
set -o pipefail # Fail if any process fails

echo "== Testing Fast Fourier Transform Lib =="
gcc -I../src ../src/fft.c test_fft.c -o test_ffc
./test_ffc

echo "== Testing dynamic memory allocation lib =="
gcc -I../src ../src/heap.c HeapTestMain.c -o test_heap
./test_heap 