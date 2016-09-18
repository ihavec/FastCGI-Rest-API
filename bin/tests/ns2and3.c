#include "ns2and3.h"
#include <stdlib.h>
FRA_NAMESPACE( ns2 );
FRA_VARIABLES(
		ns2,
		rc, int, NULL,
		name, char *, NULL
	     );


static void helper( void * * my_store ) {

	name = "nc";
	rc = -22;

}

static int get_rc( void * * my_store ) {
	return rc;
}

static char * get_name( void * * my_store ) {
	return name;
}

FRA_NAMESPACE( ns3 );
FRA_VARIABLES(
		ns3,
		rc, int, NULL,
		name, char *, NULL,
		final_return, int, NULL
	     );

static void helper2( void * * my_store ) {

	rc = get_rc( my_store ) + 44;
	name = malloc( 3 * sizeof( char ) );
	name[0] = get_name( my_store )[1];
	name[1] = get_name( my_store )[0];
	name[2] = get_name( my_store )[2];

}

int test2( void * * my_store ) {

	helper( my_store );
	helper2( my_store );
	if( rc == 22 && name[0] == 'c' && name[1] == 'n' && name[2] == '\0' ) {
		final_return = 0;
	} else {
		final_return = 1;
	}
	free( name );
	return final_return;

}
