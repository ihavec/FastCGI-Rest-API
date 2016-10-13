#include "endpoint.h"
#include <fra/core.h>

#include "var.h"

#include <stdlib.h>




// public functions

fra_endpoint_t * fra_endpoint_new() {
	return (fra_endpoint_t *)malloc( sizeof( fra_endpoint_t ) );
}

int fra_endpoint_destroy( fra_endpoint_t * e ) {

	free( e );

	return 0;

}

int fra_endpoint_hook_register(
		fra_endpoint_t * endpoint,
		enum fra_endpoint_hook_type type,
		int (*callback)( fra_req_t * ),
		float priority
		) {

	return 0;

}
