#include <fra/core.h>
#include "../src/core/dbg.h"




static int hello_world( fra_req_t * req ) {

	debug( "Hello world :)" );

	return 0;

}

static int hello_world2( fra_req_t * req ) {

	debug( "Behehehehe" );

	return 0;

}

static int hello_world3() {

	debug( "Hello 3" );

	return 0;

}

static int hello_world4() {

	debug( "Hello 4" );

	return 0;

}

static int hello_world5() {

	debug( "Hello 5" );

	return 0;

}

int main( int argc, char * * argv ) {

	int rc;


	freopen( argv[1], "w", stdout );
	freopen( argv[2], "w", stderr );

	debug( "before init" );
	rc = fra_glob_init();
	check( rc == 0, final_cleanup );

	rc = fra_req_hook_register( FRA_REQ_NEW, hello_world, 0.1f );
	check( rc == 0, final_cleanup );

	rc = fra_glob_hook_register( FRA_REQ_INCOMING, hello_world3, 5343.3f );
	check( rc == 0, final_cleanup );

	rc = fra_glob_hook_register( FRA_REQ_INCOMING, hello_world4, 543.3f );
	check( rc == 0, final_cleanup );

	rc = fra_req_hook_register( FRA_REQ_NEW, hello_world2, 0.099f );
	check( rc == 0, final_cleanup );

	rc = fra_glob_hook_register( FRA_REQ_INCOMING, hello_world5, 5343.3f );
	check( rc == 0, final_cleanup );

	debug( "after init" );

	fra_glob_poll();

	return 0;

final_cleanup:
	return -1;

}
