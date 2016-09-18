#!/bin/sh

cc -c ../../libs/bstrlib/bstrlib.c -o bstrlib.o
cc -I../../libs/bstrlib -I../../libs -g -Wall -Wextra -pedantic -std=gnu99 -o ../fra_parse fra_parse.c bstrlib.o
rm bstrlib.o
