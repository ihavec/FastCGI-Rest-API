#include "end.h"
#include <fra/core.h>

#include "var.h"
#include "ht.h"
#include "dbg.h"
#include "hook.h"
#include "lock.h"
#include "req.h"
#include "config.h"

#include <stdlib.h>
#ifndef NO_PTHREADS
#include <pthread.h>
#endif




// semi-private functions

int fra_p_end_store_set( fra_req_t * r ) {

	int rc;

	fra_p_end_store_t * cur;
	fra_end_t * e;


	check( r && r->endpoint, final_cleanup );

	e = r->endpoint;

	fra_p_lock( &e->lock, final_cleanup );

	if( ! e->store_empty ) {

		cur = malloc( sizeof( fra_p_end_store_t ) + e->store_size );
		check( cur, unlock_cleanup );

		cur->next = NULL;
		cur->store = (char *)cur + sizeof( fra_p_end_store_t );

		r->endpoint_store = cur;

		rc = fra_p_req_hook_execute( r, FRA_END_STORE_CREATED );
		check( rc == 0, cur_cleanup );

		e->store_all_count++;

	} else {

		r->endpoint_store = e->store_empty;

		e->store_empty = e->store_empty->next;

		e->store_empty_count--;

	}

	fra_p_unlock( &e->lock, unlock_cleanup );

	return 0;

cur_cleanup:
	free( cur );

unlock_cleanup:
#ifndef NO_PTHREADS
	fra_p_unlock( &e->lock, final_cleanup );

final_cleanup:
#endif
	return -1;

}

int fra_p_end_store_maybe_free( fra_req_t * r ) {

	int rc;

	fra_end_t * e;


	if( r && r->endpoint ) {

		e = r->endpoint;

		fra_p_lock( &e->lock, final_cleanup );

		if( e->store_all_count - e->store_empty_count > e->store_all_count * FRA_CORE_WAITING_ENDPOINT_STORES_GROWTH_FACTOR ) {

			rc = fra_p_req_hook_execute( r, FRA_END_STORE_FREE );
			check( rc == 0, unlock_cleanup );

			free( r->endpoint_store );
			e->store_all_count--;

		} else {

			r->endpoint_store->next = e->store_empty;
			e->store_empty = r->endpoint_store;
			e->store_empty_count++;

		}

		fra_p_unlock( &e->lock, unlock_cleanup );

	}

	return 0;

unlock_cleanup:
#ifndef NO_PTHREADS
	fra_p_unlock( &e->lock, final_cleanup );

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

	e->store_size = 0;

	e->store_empty = NULL;
	e->store_empty_count = 0;
	e->store_all_count = 0;

	e->hooks = calloc( FRA_HOOK_COUNT, sizeof( fra_p_hook_t * ) );
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

int fra_end_destroy( fra_end_t * e ) {

#ifndef NO_PTHREADS
	int rc;
#endif

	fra_p_end_store_t * cur;
	fra_req_t r;


	if( e ) {

		fra_p_lock( &e->lock, final_cleanup );

		while( e->store_empty ) {

			cur = e->store_empty;

			e->store_empty = e->store_empty->next;

			r.endpoint = e;
			r.fcgx_defined = 0;
			r.endpoint_store = cur;

			rc = fra_p_req_hook_execute( &r, FRA_END_STORE_FREE );
			check( rc == 0, unlock_cleanup );

			free( cur );

		}

		fra_p_ht_free( e->store_map );

		fra_p_hook_free( e->hooks, FRA_HOOK_COUNT );
		free( e->hooks );

#ifndef NO_PTHREADS
		fra_p_unlock( &e->lock, e_cleanup );
		rc = pthread_mutex_destroy( &e->lock );
		check( rc == 0, e_cleanup );
#endif
		free( e );

	}

	return 0;

unlock_cleanup:
	fra_p_unlock( &e->lock, e_cleanup );

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
