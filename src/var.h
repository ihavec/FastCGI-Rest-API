#ifndef fra_p_core_var_h
#define fra_p_core_var_h


#include <stddef.h>
#include <bstrlib.h>


#pragma GCC visibility push(hidden)


typedef struct {
	bstring name;
	bstring type;
	size_t position;
} fra_p_var_t;

int fra_p_var_init( int var_count );


#pragma GCC visibility pop


#endif
