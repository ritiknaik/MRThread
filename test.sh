#!/bin/sh

echo "COMPILING FILES"
echo
make
echo "TESTING ONE-ONE"
echo
cd one-one
./test
./test_spin
./test_mutest
./test_search

echo
echo
echo "TESTING MANY-ONE"
echo
cd ../many-one/
./test
echo
./test_search

cd ..
make clean
echo
echo "DONE TESTING"