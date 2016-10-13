#!/bin/sh

BUILD_DIR=$1
shift

# iterate
while test ${#} -gt 0
do
	mkdir -p $(dirname $1)
	shift
done
