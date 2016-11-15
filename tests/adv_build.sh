#!/bin/bash

t=$1
cc $(./get_cflags.sh) $t.c ../build/libfra.a -ldl -o $t-test
