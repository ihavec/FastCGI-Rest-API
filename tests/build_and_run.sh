#!/bin/bash

for t in ht end url var conf
do
	./run_simple.sh $t
	if [ $? -ne 0 ]
	then
		exit $?
	fi
done

for t in glob_var end_var var_types hook req
do
	./run_server.sh $t
	if [ $? -ne 0 ]
	then
		exit $?
	fi
done

for t in pl
do
	echo "++++++++++====================++++++++++"
	echo "-----> Running test \"$t\" ..."
	if ./$t.sh
	then
		echo "**** \"$t\" test succeded :)"
	else
		echo "!!!! \"$t\" test failed :("
		exit $?
	fi
done

echo "###############################################################"
echo "###############################################################"
echo "##############       ALL TESTS SUCCEEDED :)    ################"
echo "###############################################################"
echo "###############################################################"
