#!/bin/bash

export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:$(pwd)/pl/ldpath_dir
./adv_build.sh pl
cc -shared -o pl/ldpath_dir/libfra_test.so pl/ldpath_dir/fra_test.c
cc -shared -o pl/libfra_test2.so pl/fra_test2.c
valgrind --log-file=pl.valgrind.log --leak-check=full --show-leak-kinds=all --error-exitcode=2 ./pl-test \
	&& grep -q "in use at exit: 0 bytes in 0 blocks" pl.valgrind.log
