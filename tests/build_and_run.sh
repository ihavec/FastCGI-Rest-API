#!/bin/bash

for t in ht end url var
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
	if valgrind --log-file=$t.valgrind.log --leak-check=full --show-leak-kinds=all --error-exitcode=2 ./$t-test \
		&& grep -q "in use at exit: 0 bytes in 0 blocks" $t.valgrind.log
	then
		echo "**** Valgrind test succeded"
	else
		echo "!!!! Valgrind test failed"
		exit -2
	fi
done

for t in glob_var end_var var_types hook
do
	cc -I../include -I../libs/bstrlib -Wall -Wextra -pedantic -std=gnu99 -g $t.c ../build/libfra.a -lfcgi -o test
	echo "++++++++++====================++++++++++"
	echo "-----> Running test \"$t\" ..."
cat > test.conf <<EOF
	server.document-root = "/var/empty"
	server.port = 8080
	server.modules += ( "mod_rewrite" )
	url.rewrite-if-not-file = ( "(.*)" => "/" )
	server.modules += ( "mod_fastcgi" )
	fastcgi.server = ( "/" => ((
	"socket" => "$(pwd)/test.sock"
	))
	)
EOF
	valgrind --suppressions=fake_fcgx_deinit.valgrind.suppression --log-file=test.valgrind.log --trace-children=yes --leak-check=full --show-leak-kinds=all --error-exitcode=2 -- spawn-fcgi -P test.pid -s test.sock ./test > /dev/null 2>&1 &
	disown
	#allow valgrind time to setup the server...
	sleep 0.3
	lighttpd -D -f test.conf > /dev/null 2>&1 &
	pid=$!
	disown
	url=$(<$t.url)
	curl -s localhost:8080${url} > test.result
	for i in {0..20}
	do
		if [ ! -f $t.$i.url ]
		then
			break
		fi
		curl -s localhost:8080$(<$t.$i.url) > test.$i.result
	done
	curl -s localhost:8080/die > /dev/null
	#allow the server time to die
	sleep 0.3
	kill $pid
	if diff test.result $t.expected
	then
		echo "**** \"$t\" test succeded :)"
	else
		echo "!!!! \"$t\" test failed :("
		exit -1
	fi
	for i in {0..20}
	do
		if [ ! -a $t.$i.expected ]
		then
			break
		fi
		if diff test.$i.result $t.$i.expected
		then
			echo "**** \"$t.$i\" test succeded :)"
		else
			echo "!!!! \"$t.$i\" test failed :("
			exit -1
		fi
	done
	echo "-----> Running valgrind for test \"$t\" ..."
	grep -q "in use at exit: 0 bytes in 0 blocks" test.valgrind.log
	ex1=$?
	grep -q " definitely lost: 0 bytes in 0 blocks" test.valgrind.log \
		&& grep -q "indirectly lost: 0 bytes in 0 blocks" test.valgrind.log \
		&& grep -q "possibly lost: 0 bytes in 0 blocks" test.valgrind.log \
		&& grep -q "still reachable: 0 bytes in 0 blocks" test.valgrind.log
	ex2=$?

	if [ $ex1 -eq 0 -o $ex2 -eq 0 ]
	then
		echo "**** Valgrind test succeded"
	else
		echo "!!!! Valgrind test failed"
		exit -2
	fi
done
