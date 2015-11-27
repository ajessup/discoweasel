set -e
set -o pipefail # Fail if any process fails
gcc -I../src ../src/fft.c test_fft.c -o test_ffc
./test_ffc