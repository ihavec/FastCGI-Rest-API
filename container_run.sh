#!/bin/sh

#if [ ! -f /fra-server/example/build/fra-server-example ] ; then
	cd /fra-server/example
	make dev
#fi

sleep 5000

#spawn-fcgi -p 8008 -n -- /fra-server/example/build/fra-server-example /fra-server/example/build/frar-server-example.log /fra-server/example/build/fra-server-example.err
