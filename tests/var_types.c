#include <fra/core.h>
#include "../src/dbg.h"
#include <bstrlib.h>




struct for_nesting {
	char * value;
};

struct my_struct {
	int my_int;
	long my_long;
	struct for_nesting nest;
	char * name;
};

typedef struct for_nesting nnn;

typedef void (*ccc)( int, float, fra_req_t * );

struct wrapper {
	ccc buhu;
};

static fra_end_t * e;

static void set_float( int first_one, float f, fra_req_t * r ) {

	if( first_one ) {

		fra( r, "firsty", float ) = f;

	} else {

		fra( r, "lasty", float ) = f;

	}

}

static int set_vars( fra_req_t * req ) {

	if( fra_req_endpoint( req ) == e ) {

		fra( req, "buhu", int ) = 188;

		fra( req, "price", double ) = 99.88;

		fra( req, "my", struct my_struct ).nest.value = "It works :)";
		fra( req, "my", struct my_struct ).name = "Or doesn't it :( ?";
		fra( req, "my", struct my_struct ).my_int = -111;
		fra( req, "my", struct my_struct ).my_long = LONG_MIN;

		fra( req, "your", nnn ).value = "Ojeeej";

		fra( req, "call", ccc )( 1, 8.181f, req );

		fra( req, "call_wrap", struct wrapper ).buhu( 0, 7.181f, req );

	} else {

		debug( "Something else called" );

	}

	return 0;

}

static int create_vars( fra_req_t * r ) {

	if( fra_req_endpoint( r ) == e ) {

		fra( r, "book", bstring ) = bfromcstr( "A great one" );
		check( fra( r, "book", bstring ), final_cleanup );

		fra( r, "failure", struct tagbstring ).data = (unsigned char *)"bebebe";
		fra( r, "failure", struct tagbstring ).slen = sizeof( "bebebe" ) - 1;
		fra( r, "failure", struct tagbstring ).mlen = -1;

		fra( r, "currency", char * ) = "EUR";

		fra( r, "call", ccc ) = set_float;

		fra( r, "call_wrap", struct wrapper ).buhu = set_float;

	}

	return 0;

final_cleanup:
	return -1;

}

static int destroy_vars( fra_req_t * r ) {

	if( fra_req_endpoint( r ) == e ) {

		bdestroy( fra( r, "book", bstring ) );

	}

	return 0;

}

static int handle( fra_req_t * req ) {

	FCGX_FPrintF(
			fra_req_fcgx( req )->out,
			"Status: 200 OK\n"
			"Content-type: application/json; charset=utf-8\n"
			"\n"
			"buhu %d\n"
			"The book %s costs %f%s\n"
			"%.*s\n"
			"my_struct dump:\n"
			"{\n"
			"my_int: %d;\n"
			"my_long: %ld;\n"
			"nest.value: %s;\n"
			"name: %s;\n"
			"}\n"
			"%s\n"
			"call produced: %.3f\n"
			"call_wrap produced: %.3f\n"
			"\n",
			fra( req, "buhu", int ),
			bdata( fra( req, "book", bstring) ),
			fra( req, "price", double ),
			fra( req, "currency", char * ),
			blength( &fra( req, "failure", struct tagbstring ) ),
			bdata( &fra( req, "failure", struct tagbstring ) ),
			fra( req, "my", struct my_struct ).my_int,
			fra( req, "my", struct my_struct ).my_long,
			fra( req, "my", struct my_struct ).nest.value,
			fra( req, "my", struct my_struct ).name,
			fra( req, "your", nnn ).value,
			fra( req, "firsty", float ),
			fra( req, "lasty", float )
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

	rc = fra_req_hook_reg( FRA_END_STORE_CREATED, create_vars, 0.099f );
	check( rc == 0, final_cleanup );

	rc = fra_req_hook_reg( FRA_END_STORE_FREE, destroy_vars, 0.099f );
	check( rc == 0, final_cleanup );

	e = fra_end_new( 20 );
	check( e, final_cleanup );

	debug_v( "Endpoint is %p", (void *)e );

	rc = fra_reg( e, "buhu", int );
	check( rc == 0, final_cleanup );

	rc = fra_reg( e, "book", bstring );
	check( rc == 0, final_cleanup );

	rc = fra_req_reg( "currency", char * );
	check( rc == 0, final_cleanup );

	rc = fra_reg( e, "price", double );
	check( rc == 0, final_cleanup );

	rc = fra_reg( e, "failure", struct tagbstring );
	check( rc == 0, final_cleanup );

	rc = fra_reg( e, "my", struct my_struct );
	check( rc == 0, final_cleanup );

	rc = fra_reg( e, "your", nnn );
	check( rc == 0, final_cleanup );

	rc = fra_reg( e, "firsty", float );
	check( rc == 0, final_cleanup );

	rc = fra_reg( e, "lasty", float );
	check( rc == 0, final_cleanup );

	// This should produce a warning on a good enough compiler following the C standard
	// as libfra internally of course uses void * for the hashtables
	// It should work on most systems in use today as normal pointers
	// have the same size as function pointers but doesn't follow the standard
	rc = fra_reg( e, "call", ccc );
	check( rc == 0, final_cleanup );

	// To follow the standard and still use functions pointers with fra_reg() and fra()
	// you can use a wrapper struct
	rc = fra_reg( e, "call_wrap", struct wrapper );
	check( rc == 0, final_cleanup );

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
