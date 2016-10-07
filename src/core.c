#include <fra/core.h>

#include "var.h"

#include <stddef.h>
#include <bstrlib.h>




struct fra_endpoint {
	bstring name;
	size_t store_size;
	fra_p_hashtable_t store_map;
};

struct fra_req {
	void * store;
	fra_endpoint_t * endpoint;
};

int fra_register( fra_endpoint_t * endpoint, char * name, const char * type, size_t size ) {

	int rc;
	fra_p_var_t * var;


	check( endpoint->store_size < SIZE_MAX - size, final_cleanup );

	var = malloc( sizeof( fra_p_var_t ) );
	check( var, final_cleanup );

	var->name = bfromcstr( name );
	check( var->name, var_cleanup );

	var->type = bfromcstr( type );
	check( var->type, name_cleanup );

	var->position = store_size;

	rc = fra_p_hashtable_set( endpoint->store_map, name, var );
	check_msg( rc != 1, type_cleanup, "You have already registered a variable with the same name for this endpoint." );
	check( rc == 0, type_cleanup );

	endpoint->store_size += size;

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

int fra_p_req_init( fra_endpoint_t * e ) {

	fra_req_t * req;


	/* ... */
	req = malloc( sizeof( fra_req_t ) );
	req->store = malloc( e->store_size );
	/* ... */

	call_user_on_req_init_hook( e );

	return 0;

}

//user hooks that can be registered to endpoint (with float priority value for before or after (authentication uses this with prioriti 10.0f)):
//on request created
//on request reset (request reused for another request)

int user_on_req_init_hook_example( fra_req_t * req ) {

	fra_i( req, "rc" ) = 0;

	fra( req, "message", bstring ) = bfromcstr( "Hello world!" );
	check( fra( req, "message", bstring ), final_cleanup );

	return 0; //must return 0 or else the request is not created...

final_cleanup:
	return -1;

}

void * fra_var_get( fra_req_t * request,  char * name, const char * type ) {

	void * position;
	fra_p_var_t * var;


	var = (fra_p_var_t *)fra_p_hashtable_get( request->endpoint->store_map, name );
	check_msg( var, final_cleanup, "No variable \"%s\" found for endpoint \"%s\"", name, request->endpoint->name );
	check_msg(
			biseqcstr( var->type, type ) == 0,
			final_cleanup,
			"Wrong type specified when getting variable \"%s\" from endpoint \"%s\"\n"
			"Correct type is \"%s\" but you wrote \"%s\"",
			name,
			request->endpoint->name,
			bdata( var->type ),
			type
		 );

	position = request->store + var->position;

	return position;

final_cleanup:
	return NULL;

}

fra_endpoint_t * fra_endpoint_new() {
	return (fra_endpoint_t *)malloc( sizeof( fra_endpoint_t ) );
}

int fra_endpoint_destroy( fra_endpoint_t * e ) {

	free( e );

	return 0;

}

int fra_endpoint_hook_register(
		fra_endpoint_t * endpoint,
		enum fra_hook_type type,
		int (*callback)( fra_req_t * ),
		float priority
		) {

	return 0;

}

