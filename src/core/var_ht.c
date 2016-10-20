#include "var_ht.h"
#include "dbg.h"
#include "var.h"

#include "murmur3.h"
#include <stdint.h>
#include <stdlib.h>




struct bucket {
	int el_c;
	fra_p_var_t * el;
};

struct hashtable {
	int collisions_count;
	int buck_c;
	struct bucket * buck;
};

fra_p_var_ht_t * fra_p_var_ht_create( int bucket_count ) {

	fra_p_var_ht_t * ht;


	ht = malloc( sizeof( struct hashtable ) );
	check( ht, final_cleanup );

	ht->collisions_count = 0;

	ht->buck_c = bucket_count;

	ht->buck = calloc( ht->buck_c, sizeof( struct bucket ) );
	check( ht->buck, ht_cleanup );

	return ht;

ht_cleanup:
	free( ht );

final_cleanup:
	return NULL;

}

int fra_p_var_ht_set( fra_p_var_ht_t * ht, fra_p_var_t * var ) {

	uint32_t hash;
	unsigned int i;
	void * tmp;


	MurmurHash3_x86_32( (void *)var->name->data, var->name->slen, 55, &hash );

	i = hash % ht->buck_c;

	tmp = realloc( ht->buck[i].el, ( ht->buck[i].el_c + 1 ) * sizeof( fra_p_var_t ) );
	check( tmp, final_cleanup );
	ht->buck[i].el = tmp;

	ht->buck[i].el[ ht->buck[i].el_c ] = *var;

	ht->buck[i].el_c++;
	if( ht->buck[i].el_c > 1 ) ht->collisions_count++;
	if( ht->collisions_count > (int)( (float)ht->buck_c * 0.1 )  ) {
		log_warn(
				"There are more than 10%% collisions in the wariables hashtable.\n"
				"To improve performance consider setting the max_vars_count in the \n"
				"fra_glob_init() call to a higher value."
			);
		ht->collisions_count = INT_MIN;
	}

	return 0;

final_cleanup:
	return -1;

}

fra_p_var_t * fra_p_var_ht_get( fra_p_var_ht_t * ht, const char * name, int name_len, uint32_t hash ) {

	unsigned int i;
	int j;
	struct tagbstring name_bstr;


	check( ht, final_cleanup );

	name_bstr.mlen = -1;
	name_bstr.slen = name_len - 1;
	name_bstr.data = (unsigned char *)name;

	i = hash % ht->buck_c;

	for( j = 0; j < ht->buck[i].el_c; j++ ) {
		if( biseq( ht->buck[i].el[j].name, &name_bstr ) == 1 ) return &ht->buck[i].el[j];
	}

	return NULL;

final_cleanup:
	return NULL;

}
