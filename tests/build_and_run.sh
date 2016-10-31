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

for t in glob_var
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
	"socket" => "$(pwd)/test.sock",
	"bin-path" => "$(pwd)/test"
	))
	)
EOF
	valgrind --log-file=test.valgrind.log --trace-children=yes --leak-check=full --show-leak-kinds=all --error-exitcode=2 -- lighttpd -D -f test.conf > /dev/null 2>&1 &
	pid=$!
	disown
	#allow valgrind time to setup the server...
	sleep 1
	url=$(<$t.url)
	curl -s localhost:8080${url} > test.result
	kill $pid
	if diff test.result $t.expected
	then
		echo "**** \"$t\" test succeded :)"
	else
		echo "!!!! \"$t\" test failed :("
		exit -1
	fi
	echo "-----> Running valgrind for test \"$t\" ..."
	if grep -q "in use at exit: 0 bytes in 0 blocks" test.valgrind.log
	then
		echo "**** Valgrind test succeded"
	else
		echo "!!!! Valgrind test failed"
		exit -2
	fi
done
