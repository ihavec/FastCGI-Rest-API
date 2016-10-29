#include "end.h"
#include <fra/core.h>

#include "var.h"
#include "ht.h"
#include "dbg.h"
#include "hook.h"
#include "lock.h"
#include "url_ht.h"

#include <stdlib.h>
#ifndef NO_PTHREADS
#include <pthread.h>
#endif




// private functions and stuff :)

#ifndef NO_PTHREADS
static pthread_mutex_t urls_lock;
#endif
//static fra_p_url_ht_t * urls;




// semi-private functions

int fra_p_end_init() {

	int rc;


#ifndef NO_PTHREADS
	if( fra_p_pthreads ) {
		rc = pthread_mutex_init( &urls_lock, NULL );
		check( rc, final_cleanup );
	}
#endif

	//urls = fra_p_url_ht_new( 500 );
	//check( urls, urls_lock_cleanup );

	return 0;

urls_lock_cleanup:
#ifndef NO_PTHREADS
	if( fra_p_pthreads ) {
		rc = pthread_mutex_init( &urls_lock, NULL );
		check( rc, final_cleanup );
	}

final_cleanup:
#endif
	return -1;

}




// public functions

fra_end_t * fra_end_new( int var_count ) {

#ifndef NO_PTHREADS
	int rc;
#endif

	fra_end_t * e;


	e = malloc( sizeof( fra_end_t ) );
	check( e, final_cleanup );

#ifndef NO_PTHREADS
	rc = pthread_mutex_init( &e->lock, NULL );
	check( rc == 0, e_cleanup );
#endif

	e->store_map = fra_p_var_ht_get( var_count );
	check( e->store_map, lock_cleanup );

	e->hooks = calloc( FRA_GLOB_HOOK_COUNT, sizeof( fra_p_hook_t * ) );
	check( e->hooks, store_map_cleanup );

	return e;

store_map_cleanup:
	fra_p_ht_free( e->store_map );

lock_cleanup:
#ifndef NO_PTHREADS
	if( fra_p_pthreads ) {
		rc = pthread_mutex_destroy( &e->lock );
		check( rc == 0, e_cleanup );
	}

e_cleanup:
#endif
	free( e );

final_cleanup:
	return NULL;

}

int fra_end_free( fra_end_t * e ) {

#ifndef NO_PTHREADS
	int rc;
#endif


	if( e ) {

		fra_p_lock( &e->lock, final_cleanup );

		fra_p_ht_free( e->store_map );
		free( e->hooks );
#ifndef NO_PTHREADS
		fra_p_lock( &e->lock, e_cleanup );
		rc = pthread_mutex_destroy( &e->lock );
		check( rc == 0, e_cleanup );
#endif
		free( e );

	}

	return 0;

#ifndef NO_PTHREADS
e_cleanup:
	free( e );
#endif

final_cleanup:
	return -1;

}

int fra_end_url_add( fra_end_t * e, char * url ) {

	int rc;


	fra_p_lock( &e->lock, final_cleanup );


	return 0;

final_cleanup:
	return -1;
}
