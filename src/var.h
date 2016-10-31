#ifndef fra_p_var_h
#define fra_p_var_h

#include "ht.h"

#include <stddef.h>
#include <bstrlib.h>


#pragma GCC visibility push(hidden)


int fra_p_var_init( int var_count );

fra_p_ht_t *  fra_p_var_ht_get( int var_count );

size_t fra_p_var_store_size_get( int * rc_r );

void fra_p_var_deinit();


#pragma GCC visibility pop


#endif
