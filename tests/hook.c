#include <fra/core.h>
#include "../src/dbg.h"




static int handle( fra_req_t * req ) {

	FCGX_FPrintF(
			fra_req_fcgx( req )->out,
			"Status: 200 OK\n"
			"Content-type: application/json; charset=utf-8\n"
			"\n"
			"Behe"
			"\n"
		    );

	return 0;

}

static int handle2( fra_req_t * req ) {

	FCGX_FPrintF(
			fra_req_fcgx( req )->out,
			"Status: 200 OK\n"
			"Content-type: application/json; charset=utf-8\n"
			"\n"
			"Buhu"
			"\n"
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

static int created( fra_req_t * r ) {

	debug( "New request created" );

	return 0;

}

static int new( fra_req_t * r ) {

	debug( "New request has come in" );

	return 0;

}

static int e_created( fra_req_t * r ) {

	debug( "e created" );

	return 0;

}

static int e_new( fra_req_t * r ) {

	debug( "e new" );

	return 0;

}

static int e2_created( fra_req_t * r ) {

	debug( "e2 created" );

	return 0;

}

static int e2_new( fra_req_t * r ) {

	debug( "e2 new" );

	return 0;

}

int main() {

	int rc;

	fra_end_t * e;
	fra_end_t * e2;
	fra_end_t * e3;
	FILE * f;
	FILE * f2;


	f = freopen( "test.log", "w", stdout );
	setlinebuf( f );
	f2 = freopen( "test.err", "w", stderr );
	setlinebuf( f2 );

	rc = fra_glob_init();
	check( rc == 0, final_cleanup );

	rc = fra_req_hook_reg( FRA_REQ_CREATED, created, 0.1f );
	check( rc == 0, final_cleanup );

	rc = fra_req_hook_reg( FRA_REQ_NEW, new, 0.11f );
	check( rc == 0, final_cleanup );

	e = fra_end_new( 20 );
	check( e, final_cleanup );

	rc = fra_end_hook_reg( e, FRA_REQ_NEW, e_new, 0.11f );
	check( rc == 0, final_cleanup );

	rc = fra_end_callback_set( e, handle );
	check( rc == 0, final_cleanup );

	rc = fra_end_url_add( e, "GET", "/print/behe" );
	check( rc == 0, final_cleanup );

	e2 = fra_end_new( 20 );
	check( e2, final_cleanup );

	rc = fra_end_callback_set( e2, finish_app );
	check( rc == 0, final_cleanup );

	rc = fra_end_url_add( e2, "GET", "/die" );
	check( rc == 0, final_cleanup );

	e3 = fra_end_new( 20 );
	check( e3, final_cleanup );

	rc = fra_end_hook_reg( e3, FRA_REQ_NEW, e2_new, 0.11f );
	check( rc == 0, final_cleanup );

	rc = fra_end_callback_set( e3, handle2 );
	check( rc == 0, final_cleanup );

	rc = fra_end_url_add( e3, "GET", "/print/buhu" );
	check( rc == 0, final_cleanup );

	rc = fra_glob_poll();
	check( rc == 0, final_cleanup );

	fra_end_free( e );

	fra_end_free( e2 );

	fra_end_free( e3 );

	fra_glob_deinit();

	debug( "All cleaned up :)" );

	fclose( f );
	fclose( f2 );

	return 0;

final_cleanup:
	return -1;

}
