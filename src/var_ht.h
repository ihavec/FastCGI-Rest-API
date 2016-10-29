#ifndef fra_p_core_var_ht_h
#define fra_p_core_var_ht_h

#include "var.h"

#include <stdint.h>




#pragma GCC visibility push(hidden)

typedef struct hashtable fra_p_var_ht_t;

fra_p_var_ht_t * fra_p_var_ht_new( int bucket_count );

int fra_p_var_ht_set( fra_p_var_ht_t * ht, const char * name, const char * type, size_t pos );

fra_p_var_t * fra_p_var_ht_get( fra_p_var_ht_t * ht, const char * name, int name_len, uint32_t hash );

void fra_p_var_ht_free();

#pragma GCC visibility pop




#endif
