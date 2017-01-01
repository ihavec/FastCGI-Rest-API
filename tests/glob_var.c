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

static int finish_app( fra_req_t * req ) {

	int rc;


	FCGX_FPrintF(
			fra_req_fcgx( req )->out,
			"Status: 200 OK\n"
			"Content-type: application/json; charset=utf-8\n"
			"\n"
			"Will now stop the server"
			"\n"
		    );

	rc = fra_glob_poll_stop();
	check( rc == 0, final_cleanup );

	return 0;

final_cleanup:
	return -1;

}

int main() {

	int rc;

	fra_end_t * e;
	fra_end_t * e2;
	FILE * f;
	FILE * f2;


	f = freopen( "test.log", "w", stdout );
	setlinebuf( f );
	f2 = freopen( "test.err", "w", stderr );
	setlinebuf( f2 );

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

	e2 = fra_end_new( 20 );
	check( e, final_cleanup );

	rc = fra_end_callback_set( e2, finish_app );
	check( rc == 0, final_cleanup );

	rc = fra_end_url_add( e2, "GET", "/die" );
	check( rc == 0, final_cleanup );

	rc = fra_glob_poll();
	check( rc == 0, final_cleanup );

	fra_end_free( e );

	fra_end_free( e2 );

	fra_glob_deinit();

	debug( "All cleaned up :)" );

	fclose( f );
	fclose( f2 );

	return 0;

final_cleanup:
	return -1;

}
