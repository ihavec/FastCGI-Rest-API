#include <fra/core.h>
#include "hook.h"

#include "end.h"
#include "dbg.h"
#include "req.h"
#include "lock.h"

#include <stdlib.h>
#include <pthread.h>




// private functions and stuff ... :)

typedef int (*glob_hook_call_t)();
typedef int (*req_hook_call_t)( fra_req_t * );

#ifndef NO_PTHREADS
static pthread_mutex_t glob_hooks_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t req_hooks_lock = PTHREAD_MUTEX_INITIALIZER;
#endif
static fra_p_hook_t * * glob_hooks;
static fra_p_hook_t * * req_hooks;

static int hook_reg(
#ifndef NO_PTHREADS
		pthread_mutex_t * lock,
#endif
		fra_p_hook_t * * hooks,
		int type,
		req_hook_call_t callback,
		float priority,
		int type_max
		) {

	int rc;

	fra_p_hook_t * new_hook;
	fra_p_hook_t * cur;


	check_msg( type < type_max, final_cleanup, "You can't register a hook for this type" );
	check_msg( callback, final_cleanup, "You can't register a hook with callback equal to NULL" );

	new_hook = malloc( sizeof( fra_p_hook_t ) );
	check( new_hook, final_cleanup );

	new_hook->callback = callback;
	new_hook->priority = priority;

	fra_p_lock( lock, new_hook_cleanup );

	if( ! hooks[ type ] ) {

		new_hook->next = NULL;
		hooks[ type ] = new_hook;

	} else {

		if( priority < hooks[ type ]->priority ) {

			new_hook->next = hooks[ type ];
			hooks[ type ] = new_hook;

		} else {

			for( cur = hooks[ type ]; cur; cur = cur->next ) {

				if( ( ! cur->next ) || priority < cur->next->priority  ) {

					new_hook->next = cur->next;
					cur->next = new_hook;
					break;

				}

			}

		}

	}

	fra_p_unlock( lock, final_cleanup );

	return 0;

new_hook_cleanup:
	free( new_hook );

final_cleanup:
	return -1;

}

static int hook_execute(
#ifndef NO_PTHREADS
		pthread_mutex_t * lock,
#endif
		fra_p_hook_t * * hooks,
		int type,
		int type_max,
		int put_req,
		fra_req_t * req
		) {

	int rc;

	req_hook_call_t call;
	fra_p_hook_t * cur;


	check( type < type_max && hooks, final_cleanup );

	fra_p_lock( lock, final_cleanup );

	for( cur = hooks[ type ]; cur; cur = cur->next ) {

		call = cur->callback;
		check( call, unlock_cleanup );

		fra_p_unlock( lock, final_cleanup );

		if( put_req ) {

			rc = call( req );
			check( rc == 0, final_cleanup );

		} else {

			rc = ( (glob_hook_call_t)call )();
			check( rc == 0, final_cleanup );

		}

		fra_p_lock( lock, final_cleanup );

	}

	fra_p_unlock( lock, final_cleanup );

	return 0;

unlock_cleanup:
	fra_p_unlock( lock, final_cleanup );

final_cleanup:
	return -1;

}




// semi-private functions

int fra_p_hook_init() {

	glob_hooks = calloc( FRA_GLOB_HOOK_COUNT, sizeof( fra_p_hook_t * ) );
	check( glob_hooks, final_cleanup );

	req_hooks = calloc( FRA_HOOK_COUNT, sizeof( fra_p_hook_t * ) );
	check( req_hooks, final_cleanup );

	return 0;

final_cleanup:
	return -1;

}

int fra_p_glob_hook_execute( enum fra_glob_hook_type type ) {

	return hook_execute(
#ifndef NO_PTHREADS
			&glob_hooks_lock,
#endif
			glob_hooks,
			(int)type,
			FRA_GLOB_HOOK_COUNT,
			0,
			NULL
			);

}

int fra_p_req_hook_execute( fra_req_t * req, enum fra_hook_type type ) {

	return hook_execute(
#ifndef NO_PTHREADS
			&req_hooks_lock,
#endif
			req_hooks,
			(int)type,
			FRA_HOOK_COUNT,
			1,
			req
			);

}

int fra_p_endpoint_hook_execute( fra_req_t * req, enum fra_hook_type type ) {

	check( req && req->endpoint, final_cleanup );

	return hook_execute(
#ifndef NO_PTHREADS
			NULL,
#endif
			req->endpoint->hooks,
			(int)type,
			FRA_HOOK_COUNT,
			1,
			req
			);

final_cleanup:
	return -1;

}




// public functions

int fra_glob_hook_reg( enum fra_glob_hook_type type, int (*callback)(), float priority ) {

	return hook_reg(
#ifndef NO_PTHREADS
			&glob_hooks_lock,
#endif
			glob_hooks,
			(int)type,
			(req_hook_call_t)callback,
			priority,
			FRA_GLOB_HOOK_COUNT
			);

}

int fra_req_hook_reg( enum fra_hook_type type, int (*callback)( fra_req_t * ), float priority ) {

	return hook_reg(
#ifndef NO_PTHREADS
			&req_hooks_lock,
#endif
			req_hooks,
			(int)type,
			callback,
			priority,
			FRA_HOOK_COUNT
			);

}

int fra_end_hook_reg( fra_end_t * endpoint, enum fra_hook_type type, int (*callback)( fra_req_t * ), float priority ) {

	return hook_reg(
#ifndef NO_PTHREADS
			NULL,
#endif
			endpoint->hooks,
			(int)type,
			callback,
			priority,
			FRA_HOOK_COUNT
			);

}
