#include "req.h"
#include <fra/core.h>

#include "var.h"
#include "endpoint.h"
#include "dbg.h"

#include <stdlib.h>
#include <poll.h>
#include <fcgiapp.h>




// semi-private functions

int fra_p_req_init() {

	int rc;


	rc = FCGX_Init();
	check( rc == 0, final_cleanup );

	return 0;

final_cleanup:
	return -1;

}

int fra_p_req_handle_new( short revents ) {

	int rc;
	fra_req_t * req;


	debug( "Handling new request" );

	check( revents & POLLIN, final_cleanup );

	req = malloc( sizeof( fra_req_t ) );
	check( req, final_cleanup );

	FCGX_InitRequest( &req->fcgx, 0, 0 );

	rc = FCGX_Accept_r( &req->fcgx );
	check( rc >= 0, final_cleanup );

	FCGX_FPrintF(
			req->fcgx.out,
			"Status: 200 OK\r\n"
			"Content-type: application/json; charset=utf-8\r\n"
			"\r\n"
			"{s:0}"
			"\r\n"
		    );

	FCGX_Finish_r( &req->fcgx );

	return 0;

final_cleanup:
	return -1;

}
