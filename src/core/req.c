#include "req.h"
#include <fra/core.h>

#include "var.h"
#include "endpoint.h"

#include <stdlib.h>




// semi-private functions

int fra_p_req_init( fra_endpoint_t * e ) {

	fra_req_t * req;


	/* ... */
	req = malloc( sizeof( fra_req_t ) );
	req->store = malloc( e->store_size );
	/* ... */

	//call_user_on_req_init_hook( e );

	return 0;

}

int fra_p_req_handle_new( short revents ) {

	return 0;

}
