#include "ht.h"
#include "dbg.h"

#include "murmur3.h"
#include <stdint.h>
#include <stdlib.h>
#include <bstrlib.h>




#define get_key( H, I, J ) ( *( (bstring *)( H->buck[I].el + J * ( sizeof( bstring ) + H->value_size ) ) ) )

#define get_value( H, I, J ) ( (void *)( H->buck[I].el + J * ( sizeof( bstring ) + H->value_size ) + sizeof( bstring ) ) )

struct bucket {
	int el_c;
	char * el;
};

struct fra_p_ht {
	size_t value_size;
	fra_p_ht_set_value_call set_value;
	fra_p_ht_destructor_call destructor;
	int collisions_count;
	int buck_c;
	struct bucket * buck;
};

fra_p_ht_t * fra_p_ht_new( int bucket_count, size_t value_size, fra_p_ht_set_value_call set_value, fra_p_ht_destructor_call destructor ) {

	fra_p_ht_t * ht;


	ht = malloc( sizeof( struct fra_p_ht ) );
	check( ht, final_cleanup );

	ht->collisions_count = 0;

	ht->buck_c = bucket_count;

	ht->value_size = value_size;

	ht->set_value = set_value;

	ht->destructor = destructor;

	ht->buck = calloc( ht->buck_c, sizeof( struct bucket ) );
	check( ht->buck, ht_cleanup );

	return ht;

ht_cleanup:
	free( ht );

final_cleanup:
	return NULL;

}

void fra_p_ht_free( fra_p_ht_t * ht ) {

	int i;
	int j;


	if( ht ) {

		for( i = 0; i < ht->buck_c; i++ ) {

			for( j = 0; j < ht->buck[i].el_c; j++ ) {

				bdestroy( get_key( ht, i, j ) );
				ht->destructor( get_value( ht, i, j ) );

			}

			free( ht->buck[i].el );

		}

		free( ht->buck );
		free( ht );

	}

}

int fra_p_ht_set( fra_p_ht_t * ht, const char * key, void * arg ) {

	int rc;

	uint32_t hash;
	unsigned int i;
	void * tmp;
	bstring key_str;


	key_str = bfromcstr( key );
	check( key_str, final_cleanup );

	MurmurHash3_x86_32( (void *)key_str->data, key_str->slen, 55, &hash );

	i = hash % ht->buck_c;

	tmp = realloc( ht->buck[i].el, ( ht->buck[i].el_c + 1 ) * ( sizeof( bstring ) + ht->value_size ) );
	check( tmp, key_str_cleanup );
	ht->buck[i].el = tmp;

	get_key( ht, i, ht->buck[i].el_c ) = key_str;

	rc = ht->set_value( get_value( ht, i, ht->buck[i].el_c ), arg );
	check( rc == 0, key_str_cleanup );

	ht->buck[i].el_c++;
	if( ht->buck[i].el_c > 1 ) ht->collisions_count++;
	if( ht->collisions_count > (int)( (float)ht->buck_c * 0.1 )  ) {
		log_warn(
				"There are more than 10%% collisions in the hashtable.\n"
				"To improve performance consider setting the max_vars_count in the \n"
				"fra_glob_init() call to a higher value."
			);
		ht->collisions_count = INT_MIN;
	}

	return 0;

key_str_cleanup:
	bdestroy( key_str );

final_cleanup:
	return -1;

}

void * fra_p_ht_get( fra_p_ht_t * ht, const char * key, int key_len ) {

	uint32_t hash;


	MurmurHash3_x86_32( (void *)key, key_len, 55, &hash );

	return fra_p_ht_get_by_hash( ht, key, key_len, hash );

}

void * fra_p_ht_get_by_hash( fra_p_ht_t * ht, const char * key, int key_len, uint32_t hash ) {

	unsigned int i;
	int j;
	struct tagbstring key_str;


	check( ht, final_cleanup );

	key_str.mlen = -1;
	key_str.slen = key_len;
	key_str.data = (unsigned char *)key;

	i = hash % ht->buck_c;

	for( j = 0; j < ht->buck[i].el_c; j++ ) {
		if( biseq( get_key( ht, i, j ), &key_str ) == 1 ) return get_value( ht, i, j );
	}

	return NULL;

final_cleanup:
	return NULL;

}
