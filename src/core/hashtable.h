#ifndef fra_p_core_hashtable_h
#define fra_p_core_hashtable_h

#include "var.h"




#pragma GCC visibility push(hidden)

typedef struct hashtable fra_p_hashtable_t;

fra_p_var_t * fra_p_hashtable_get( fra_p_hashtable_t * ht, const char * name );

int fra_p_hashtable_set( fra_p_hashtable_t * ht, const char * name, fra_p_var_t * var );

#pragma GCC visibility pop




#endif
