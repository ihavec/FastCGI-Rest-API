#include "end.h"
#include <fra/core.h>

#include "var.h"
#include "ht.h"
#include "dbg.h"
#include "hook.h"
#include "lock.h"

#include <stdlib.h>
#ifndef NO_PTHREADS
#include <pthread.h>
#endif




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

	e->store_size = 0;

	e->hooks = calloc( FRA_GLOB_HOOK_COUNT, sizeof( fra_p_hook_t * ) );
	check( e->hooks, store_map_cleanup );

	e->callback = NULL;

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

int fra_end_callback_set( fra_end_t * e, int (*callback)( fra_req_t * ) ) {

#ifndef NO_PTHREADS
	int rc;
#endif


	fra_p_lock( &e->lock, final_cleanup );

	e->callback = callback;

	fra_p_unlock( &e->lock, final_cleanup );

	return 0;

final_cleanup:
	return -1;

}
