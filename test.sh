#!/usr/bin/env bash

echo "compiling the test.cpp ..."
g++ -Wall -Wextra -std=c++11 test.cpp -o test

echo ""
echo "run test ..."
echo ""
./test_gen.py 10000
./test

echo ""
echo "clean ..."
rm test *.txt
