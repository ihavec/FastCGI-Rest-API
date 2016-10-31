#!/bin/sh

for t in ht end url
do
	cc -I../include -I../libs/bstrlib -Wall -Wextra -pedantic -std=gnu99 -g $t.c ../build/libfra.a -o $t-test
	echo "++++++++++====================++++++++++"
	echo "-----> Running test \"$t\" ..."
	if ./$t-test
	then
		echo "**** \"$t\" test succeded :)"
	else
		echo "!!!! \"$t\" test failed :("
		exit -1
	fi
	echo "-----> Running valgrind for test \"$t\" ..."
	if valgrind --log-file=$t.valgrind.log --leak-check=full --show-reachable=yes --error-exitcode=2 ./$t-test
	then
		echo "**** Valgrind test succeded"
	else
		echo "!!!! Valgrind test failed"
		exit -2
	fi
done
