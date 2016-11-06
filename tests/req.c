#include <fra/core.h>
#include "../src/dbg.h"
#include <bstrlib.h>




static int handle( fra_req_t * req ) {

	FCGX_FPrintF(
			fra_req_fcgx( req )->out,
			"Status: 200 OK\n"
			"Content-type: application/json; charset=utf-8\n"
			"\n"
			"Create has run: %s\n"
			"New has run: %s\n",
			( fra( req, "created has run", int ) == 1 ) ? "yes" : "no",
			( fra( req, "new has run", int ) == 1 ) ? "yes" : "no"
		    );

	fra( req, "created has run", int ) = 0;
	fra( req, "new has run", int ) = 0;

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

	fra( r, "created has run", int ) = 1;

	return 0;

}

static int new( fra_req_t * r ) {

	fra( r, "new has run", int ) = 1;

	return 0;

}

#define COUNT 6

int main() {

	int rc;

	fra_end_t * e_finish;
	FILE * f;
	FILE * f2;
	fra_end_t * e[ COUNT ];
	bstring urls[ COUNT ];
	int i;


	f = freopen( "test.log", "w", stdout );
	setlinebuf( f );
	f2 = freopen( "test.err", "w", stderr );
	setlinebuf( f2 );

	rc = fra_glob_init();
	check( rc == 0, final_cleanup );

	rc = fra_req_reg( "created has run", int );
	check( rc == 0, final_cleanup );

	rc = fra_req_reg( "new has run", int );
	check( rc == 0, final_cleanup );

	rc = fra_req_hook_reg( FRA_REQ_CREATED, created, 0.1f );
	check( rc == 0, final_cleanup );

	rc = fra_req_hook_reg( FRA_REQ_NEW, new, 0.11f );
	check( rc == 0, final_cleanup );

	for( i = 0; i < COUNT; i++ ) {

		e[i] = fra_end_new( 20 );
		check( e[i], final_cleanup );

		rc = fra_end_callback_set( e[i], handle );
		check( rc == 0, final_cleanup );

		urls[i] = bformat( "/%d", i );
		check( urls[i], final_cleanup );

		rc = fra_end_url_add( e[i], "GET", bdata( urls[i] ) );
		check( rc == 0, final_cleanup );

	}

	e_finish = fra_end_new( 20 );
	check( e_finish, final_cleanup );

	rc = fra_end_callback_set( e_finish, finish_app );
	check( rc == 0, final_cleanup );

	rc = fra_end_url_add( e_finish, "GET", "/die" );
	check( rc == 0, final_cleanup );

	rc = fra_glob_poll();
	check( rc == 0, final_cleanup );

	fra_end_free( e_finish );

	for( i = 0; i < COUNT; i++ ) {
		
		fra_end_free( e[i] );

		bdestroy( urls[i] );

	}

	fra_glob_deinit();

	debug( "All cleaned up :)" );

	fclose( f );
	fclose( f2 );

	return 0;

final_cleanup:
	return -1;

}
