#ifndef fra_p_var_h
#define fra_p_var_h


#include <stddef.h>
#include <bstrlib.h>


#pragma GCC visibility push(hidden)


typedef struct {
	bstring name;
	bstring type;
	size_t position;
} fra_p_var_t;


#pragma GCC visibility pop


#endif
