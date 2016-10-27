#include "var.h"
#include <fra/core.h>

#include "req.h"
#include "end.h"
#include "dbg.h"
#include "murmur3.h"

#include <stdint.h>
#include <stdlib.h>
#ifndef NO_PHTREADS
#include <pthread.h>
#endif




// private functions and stuff

#ifndef NO_PHTREADS
static pthread_mutex_t glob_store_map_lock = PTHREAD_MUTEX_INITIALIZER;
#endif
static fra_p_var_ht_t * glob_store_map;
static size_t glob_store_size;

static int reg( fra_p_var_ht_t * store_map, size_t * store_size, char * name, const char * type, size_t size ) {

        int rc;

        fra_p_var_t * var;


        check( *store_size < SIZE_MAX - size, final_cleanup );

        var = malloc( sizeof( fra_p_var_t ) );
        check( var, final_cleanup );

        var->name = bfromcstr( name );
        check( var->name, var_cleanup );

        var->type = bfromcstr( type );
        check( var->type, name_cleanup );

        var->position = *store_size;

        rc = fra_p_var_ht_set( store_map, var );
        check_msg( rc != 1, type_cleanup, "You have already registered a variable with the same name." );
        check( rc == 0, type_cleanup );

        *store_size += size;

        return 0;

type_cleanup:
        bdestroy( var->type );

name_cleanup:
        bdestroy( var->name );

var_cleanup:
        free( var );

final_cleanup:
        return -1;

}




// semi-private functions

int fra_p_var_init( int var_count ) {

	glob_store_map = fra_p_var_ht_create( var_count );
	check( glob_store_map, final_cleanup );

	return 0;

final_cleanup:
	return -1;

}

// public functions

int fra_end_var_reg( fra_end_t * endpoint, char * name, const char * type, size_t size ) {

	return reg( endpoint->store_map, &endpoint->store_size, name, type, size );

}

int fra_req_var_reg( char * name, const char * type, size_t size ) {

	int rc;


#ifndef NO_PTHREADS
	rc = pthread_mutex_lock( &glob_store_map_lock );
	check( rc == 0, final_cleanup );
#endif

	rc = reg( glob_store_map, &glob_store_size, name, type, size );
	check( rc == 0, unlock_cleanup );

#ifndef NO_PTHREADS
	rc = pthread_mutex_unlock( &glob_store_map_lock );
	check( rc == 0, final_cleanup );
#endif

	return 0;

unlock_cleanup:
#ifndef NO_PTHREADS
	rc = pthread_mutex_unlock( &glob_store_map_lock );
	check( rc == 0, final_cleanup );

final_cleanup:
#endif
	return -1;

}

void * fra_var_get( fra_req_t * request,  char * name, int name_len, const char * type ) {

	void * position;
	fra_p_var_t * var;
	void * store;
	uint32_t hash;


	name_len = name_len - 1;
	check_msg( name_len > 0, final_cleanup, "Variable name can't be an empty string." );

	MurmurHash3_x86_32( (void *)name, name_len, 55, &hash );

	var = fra_p_var_ht_get( glob_store_map, name, name_len, hash );

	if( var ) {

		check_msg_v(
				biseqcstr( var->type, type ) == 1,
				final_cleanup,
				"Wrong type specified when getting global variable \"%s\". "
				"Correct type is \"%s\" but you wrote \"%s\"",
				name,
				bdata( var->type ),
				type
			   );

		store = request->req_store;

	} else {

		check_msg_v( request->endpoint, final_cleanup, "No global variable \"%s\" found.", name );

		var = fra_p_var_ht_get( request->endpoint->store_map, name, name_len, hash );
		check_msg_v( var, final_cleanup, "No variable \"%s\" found for endpoint \"%s\"", name, bdata( request->endpoint->name ) );

		check_msg_v(
				biseqcstr( var->type, type ) == 1,
				final_cleanup,
				"Wrong type specified when getting variable \"%s\" from endpoint \"%s\"\n"
				"Correct type is \"%s\" but you wrote \"%s\"",
				name,
				bdata( request->endpoint->name ),
				bdata( var->type ),
				type
			   );

		store = request->endpoint_store;


	}

	position = (char *)store + var->position;

	return position;

final_cleanup:
	return NULL;

}
