#ifndef fra_p_var_h
#define fra_p_var_h

#include "ht.h"

#include <stddef.h>
#include <bstrlib.h>


#pragma GCC visibility push(hidden)


int fra_p_var_init( int var_count );

fra_p_ht_t *  fra_p_var_ht_get( int var_count );


#pragma GCC visibility pop


#endif
