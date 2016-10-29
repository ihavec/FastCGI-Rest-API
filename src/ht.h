#ifndef fra_p_ht_h
#define fra_p_ht_h

#include <stdint.h>
#include <stdlib.h>




#pragma GCC visibility push(hidden)


typedef int (*fra_p_ht_set_value_call)( void * value, void * usr_arg );

typedef void (*fra_p_ht_destructor_call)( void * );

typedef struct fra_p_ht fra_p_ht_t;

fra_p_ht_t * fra_p_ht_new( int bucket_count, size_t value_size, fra_p_ht_set_value_call set_value, fra_p_ht_destructor_call destructor );

int fra_p_ht_set( fra_p_ht_t * ht, const char * key, void * arg );

void * fra_p_ht_get( fra_p_ht_t * ht, const char * key, int key_len );

void * fra_p_ht_get_by_hash( fra_p_ht_t * ht, const char * key, int key_len, uint32_t hash );

void fra_p_ht_free();


#pragma GCC visibility pop




#endif
