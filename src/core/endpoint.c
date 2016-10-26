#include "endpoint.h"
#include <fra/core.h>

#include "var.h"
#include "dbg.h"
#include "hook.h"

#include <stdlib.h>




// public functions

fra_endpoint_t * fra_endpoint_new() {

	fra_endpoint_t * e;


	e = malloc( sizeof( fra_endpoint_t ) );
	check( e, final_cleanup );

	e->hooks = calloc( FRA_GLOB_HOOK_COUNT, sizeof( fra_p_hook_t * ) );
	check( e->hooks, e_cleanup );

	return e;

e_cleanup:
	free( e );

final_cleanup:
	return NULL;

}

int fra_endpoint_destroy( fra_endpoint_t * e ) {

	free( e->hooks );
	free( e );

	return 0;

}
