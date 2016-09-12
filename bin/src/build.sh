#!/bin/sh

cc -I../../libs/bstrlib -I../../libs -g -Wall -Wextra -pedantic -std=gnu99 -o ../fra_parse fra_parse.c ../../libs/bstrlib/bstrlib.c
