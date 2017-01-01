#include <fra/core.h>
#include "../src/url.h"
#include "../src/dbg.h"




static fra_end_t * new_end( char * v, char * u ) {

	int rc;

	fra_end_t * e;


	e = fra_end_new( 100 );
	check( e, final_cleanup );

	rc = fra_end_url_add( e, v, u );
	check( rc == 0, final_cleanup );

	return e;

final_cleanup:
	return NULL;

}

static fra_end_t * get_end( char * v, char * u  ) {

	fra_end_t * e;
	bstring url;
	bstring verb;


	e = fra_p_url_to_endpoint( ( verb = bfromcstr( v ) ), ( url = bfromcstr( u ) ) );
	check( verb && url, final_cleanup );

	bdestroy( verb );
	bdestroy( url );

	return e;

final_cleanup:
	return NULL;

}

int main() {

	int rc;

	fra_end_t * e;
	fra_end_t * e2;
	fra_end_t * e3;
	fra_end_t * e4;


	rc = fra_p_url_init();
	check( rc == 0, final_cleanup );

	e = new_end( "POST", "/order" );
	check( e, final_cleanup );

	e2 = get_end( "POST", "/order" );
	check( e2 == e, final_cleanup );

	rc = fra_end_url_add( e, "GET", "/order" );
	check( rc == 0, final_cleanup );

	e2 = get_end( "GET", "/order" );
	check( e2 == e, final_cleanup );

	rc = fra_end_url_add( e, "POST", "/orders" );
	check( rc == 0, final_cleanup );

	e2 = get_end( "POST", "/orders" );
	check( e2 == e, final_cleanup );

	e2 = get_end( "PUT", "/order" );
	check( e2 == NULL, final_cleanup );

	e3 = new_end( "PUT", "/order" );
	check( e3, final_cleanup );

	e2 = get_end( "PUT", "/order" );
	check( e2 == e3, final_cleanup );

	e4 = new_end( "GET", "/buhu" );
	check( e4, final_cleanup );

	e2 = get_end( "GET", "/buhu" );
	check( e2 == e4, final_cleanup );

	e2 = get_end( "POST", "/buhu" );
	check( e2 == NULL, final_cleanup );

	fra_end_destroy( e );

	fra_end_destroy( e3 );

	fra_end_destroy( e4 );

	fra_p_url_deinit();

	return 0;

final_cleanup:
	return -1;

}
