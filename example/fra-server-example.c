#include <fra/core.h>
#include "../src/core/dbg.h"




int main( int argc, char * * argv ) {

	int rc;


	freopen( argv[1], "w", stdout );
	freopen( argv[2], "w", stderr );

	debug( "before init" );
	rc = fra_glob_init();
	check( rc == 0, final_cleanup );

	debug( "after init" );

	fra_glob_poll();

	return 0;

final_cleanup:
	return -1;

}
