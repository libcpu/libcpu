make clean
make &&
./test_6502 test/data/cbmbasic.bin `cat test/data/hints_cbmbasic.txt`
