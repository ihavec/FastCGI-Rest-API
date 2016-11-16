#include "pl.h"

#include "dbg.h"

#include <stdlib.h>
#include <dlfcn.h>




// private stuff

static fra_p_pl_t * g_first = NULL;
static fra_p_pl_t * g_last = NULL;

typedef int (*main_func_cb)( int, char * * );




// semi-private functions

void fra_p_pl_free( fra_p_pl_t * pl ) {

	int i;


	bdestroy( pl->name );
	for( i = 0; i < pl->argc; i++ ) free( pl->argv[i] );
	free( pl->argv );
	free( pl );

}

void fra_p_pl_reset() {

	fra_p_pl_t * cur;


	while( g_first ) {

		cur = g_first;
		g_first = g_first->next;

		fra_p_pl_free( cur );

	}

}

void fra_p_pl_add( fra_p_pl_t * pl ) {

	pl->next = NULL;

	if( g_first ) {

		g_last->next = pl;

	} else {

		g_first = pl;

	}

	g_last = pl;

}

int fra_p_pl_load() {

	int rc;
	char * rc_msg;

	fra_p_pl_t * c;
	main_func_cb s;


	for( c = g_first; c; c = c->next ) {

		c->h = dlopen( bdata( c->name ), RTLD_LAZY );
		check_msg_v(
				c->h, final_cleanup,
				"Loading the shared library \"%s\", failed with error: \"%s\".",
				bdata( c->name ),
				( rc_msg = dlerror() ) ? rc_msg : ""
			   );

		check_msg_v(
				c->argc > 0, h_cleanup,
				"You must specify which function to call to initialize the \"%s\" library "
				"by providing a json string array  for the \"%s\" key in the config file "
				"(first element is the function name and all the elements get passed to "
				"the init function using the int main( int argc, char * * argv ) style ).",
				bdata( c->name ),
				bdata( c->name )
		     );
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
		s = dlsym( c->h, c->argv[0] );
#pragma GCC diagnostic pop
		check_msg_v(
				s, h_cleanup,
				"Could not find the function \"%s\" in the \"%s\" library."
				"The error was: \"%s\".",
				c->argv[0],
				bdata( c->name ),
				( rc_msg = dlerror() ) ? rc_msg : ""
		     );

		rc = s( c->argc, c->argv );
		check_msg_v(
				rc == 0, h_cleanup,
				"The \"%s\" function from the \"%s\" library "
				"returned a non-zero value: %d.\n"
				"Aborting...",
				bdata( c->name ),
				bdata( c->name ),
				rc
			 );

	}

	return 0;

h_cleanup:
	rc = dlclose( c->h );
	check( rc == 0, final_cleanup );
	c->h = NULL;

final_cleanup:
	return -1;

}

int fra_p_pl_unload() {

	int rc;
	int rc_c;

	fra_p_pl_t * c;


	rc_c = 0;


	for( c = g_first; c; c = c->next ) {

		rc = dlclose( c->h );
		if( rc != 0 ) rc_c++;


	}
	check( rc_c == 0, final_cleanup );

	return 0;

final_cleanup:
	return -1;

}
