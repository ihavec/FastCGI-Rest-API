#!/bin/bash

rm -rf out
mkdir -p out/tests

cd ../src
./build.sh

cd ..
./fra_parse -o tests/out tests/*.c tests/*.h

cd tests/out/tests
cc ns1.c ns2and3.c main.c -o main

./main

rc=$?
if [ $rc -eq 0 ]
then
	echo "All tests succesfull :)"
else
	echo "Some tests failed :("
fi

cd ../..

exit $rc
