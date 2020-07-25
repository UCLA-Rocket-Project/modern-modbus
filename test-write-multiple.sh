#!/bin/bash

# RMMS doesn't provide a way to test writing multiple, so we use this script to do it
echo "You will need to supply a SIGINT to kill NC"
echo "Testing writing multiple coils. "
head -n 1 ./write-multiple.hex | xxd -p -r | nc 127.0.0.1 5020 > test-1.txt

echo "Testing writing multiple registers"
tail -n 1 ./write-multiple.hex | xxd -p -r | nc 127.0.0.1 5020 > test-2.txt

echo "writing multiple coils:"
cat test-1.txt | xxd -p
echo "writing multiple registers:"
cat test-2.txt | xxd -p

rm test-1.txt test-2.txt