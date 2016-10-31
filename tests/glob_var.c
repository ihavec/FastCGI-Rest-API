#include <fra/core.h>
#include "../src/dbg.h"




static int set_vars( fra_req_t * req ) {

	fra( req, "buhu", int ) = 188;

	return 0;

}

static int handle( fra_req_t * req ) {

	FCGX_FPrintF(
			fra_req_fcgx( req )->out,
			"Status: 200 OK\n"
			"Content-type: application/json; charset=utf-8\n"
			"\n"
			"%d"
			"\n",
			fra( req, "buhu", int )
		    );

	return 0;

}

int main() {

	int rc;

	fra_end_t * e;


	freopen( "test.log", "w", stdout );
	freopen( "test.err", "w", stderr );

	rc = fra_glob_init();
	check( rc == 0, final_cleanup );

	rc = fra_req_hook_reg( FRA_REQ_NEW, set_vars, 0.099f );
	check( rc == 0, final_cleanup );

	rc = fra_req_reg( "buhu", int );
	check( rc == 0, final_cleanup );

	e = fra_end_new( 20 );
	check( e, final_cleanup );

	rc = fra_end_callback_set( e, handle );
	check( rc == 0, final_cleanup );

	rc = fra_end_url_add( e, "GET", "/print/buhu" );
	check( rc == 0, final_cleanup );

	fra_glob_poll();

	return 0;

final_cleanup:
	return -1;

}
