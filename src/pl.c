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
	void * h;
	main_func_cb s;
	bstring s_str;


	s_str = NULL;

	for( c = g_first; c; c = c->next ) {

		h = dlopen( bdata( c->name ), RTLD_NOW );
		check_msg_v(
				h, s_str_cleanup,
				"Loading the shared library \"%s\", failed with error: \"%s\".",
				bdata( c->name ),
				( rc_msg = dlerror() ) ? rc_msg : ""
			   );

		if( s_str ) {

			rc = bassignformat( s_str, "%s_main_init", bdata( c->name ) );
			check( rc == BSTR_OK, s_str_cleanup );

		} else {

			s_str = bformat( "%s_main_init", bdata( c->name ) );
			check( s_str, s_str_cleanup );

		}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
		s = dlsym( h, (char *)s_str->data );
#pragma GCC diagnostic pop
		check_msg_v(
				s, s_str_cleanup,
				"Could not find the function \"%s\" in the \"%s\" library."
				"The error was: \"%s\".",
				bdata( s_str ),
				bdata( c->name ),
				( rc_msg = dlerror() ) ? rc_msg : ""
		     );

		rc = s( c->argc, c->argv );
		check_msg_v(
				rc == 0, s_str_cleanup,
				"The \"%s\" function from the \"%s\" library "
				"returned a non-zero value: %d.\n"
				"Aborting...",
				bdata( s_str ),
				bdata( c->name ),
				rc
			 );

	}

	bdestroy( s_str );

	return 0;

s_str_cleanup:
	bdestroy( s_str );
	return -1;

}
