#include "poll.h"
#include <fra/core.h>

#include "dbg.h"
#include "req.h"

#include <poll.h>
#ifndef NO_PTHREADS
#include <pthread.h>
#endif
#include <stdlib.h>




// private structs

struct callback {
	fra_req_t * req;
	union {
		int (*cb)( short );
		int (*req_cb)( fra_req_t *, short );
	} u;
};




// private variables

static int initialized = 0;
static struct pollfd * main_poll;
static int main_poll_len;
static int main_poll_max_len;
static struct callback * callbacks;
#ifndef NO_PTHREADS
static pthread_mutex_t main_poll_lock = PTHREAD_MUTEX_INITIALIZER;
#endif




// private functions

static int main_poll_maybe_grow() {

	void * tmp;
	int new_max;


	check( main_poll_len < main_poll_max_len, final_cleanup );

	if( main_poll_len == main_poll_max_len - 1 ) {

		new_max = (int)( main_poll_max_len * 1.618f );
		check( new_max > main_poll_max_len, final_cleanup );

		tmp = realloc( main_poll, new_max * sizeof( struct pollfd ) );
		check( tmp, final_cleanup );

		main_poll = tmp;

		tmp = realloc( callbacks, new_max * sizeof( struct pollfd ) );
		check( tmp, final_cleanup );

		callbacks = tmp;

		main_poll_max_len = new_max;

	}

	return 0;

final_cleanup:
	return -1;

}

static int fd_add( int fd, short events, int (*cb)( short ), int (*req_cb)( fra_req_t *, short ), fra_req_t * req ) {

	int rc;


#ifndef NO_PTHREADS
	rc = pthread_mutex_lock( &main_poll_lock );
	check( rc == 0, final_cleanup );
#endif

	check( initialized, unlock_cleanup );

	rc = main_poll_maybe_grow();
	check( rc == 0, unlock_cleanup );

	main_poll[ main_poll_len ].fd = fd;
	main_poll[ main_poll_len ].events = events;

	callbacks[ main_poll_len ].req = req;
	if( req ) {
		callbacks[ main_poll_len ].u.req_cb = req_cb;
	} else {
		callbacks[ main_poll_len ].u.cb = cb;
	}

	main_poll_len++;

	return 0;

unlock_cleanup:
#ifndef NO_PTHREADS
	pthread_mutex_unlock( &main_poll_lock );

final_cleanup:
#endif
	return -1;

}





// semi-private functions

int fra_p_poll_init() {

	int rc;


#ifndef NO_PTHREADS
	rc = pthread_mutex_lock( &main_poll_lock );
	check( rc == 0, final_cleanup );
#endif

	if( ! initialized ) {

		main_poll = malloc( 10 * sizeof( struct pollfd ) );
		check( main_poll, unlock_cleanup );

		callbacks = malloc( 10 * sizeof( int ) );
		check( callbacks, main_poll_cleanup );

		main_poll_len = 0;
		main_poll_max_len = 10;

		rc = fra_glob_fd_add( 0, POLLIN, fra_p_req_handle_new );
		check( rc == 0, callbacks_cleanup );

		initialized = 1;

	} else {

		check( 0, unlock_cleanup );

	}

#ifndef NO_PTHREADS
	rc = pthread_mutex_unlock( &main_poll_lock );
	check( rc == 0, final_cleanup );
#endif

	return 0;

callbacks_cleanup:
	free( callbacks );

main_poll_cleanup:
	free( main_poll );

unlock_cleanup:
#ifndef NO_PTHREADS
	pthread_mutex_unlock( &main_poll_lock );

final_cleanup:
#endif
	return -1;

}

// public functions

int fra_req_fd_add( fra_req_t * req, int fd, short events, int (*cb)( fra_req_t *, short ) ) {

	return fd_add( fd, events, NULL, cb, req );

}

int fra_glob_fd_add( int fd, short events, int (*cb)( short ) ) {

	return fd_add( fd, events, cb, NULL, NULL );

}

int fra_glob_poll() {

	int rc;
	int i;


#ifndef NO_PTHREADS
	rc = pthread_mutex_lock( &main_poll_lock );
	check( rc == 0, final_cleanup );
#endif

	check( initialized, unlock_cleanup );

	while( 1 ) {

		rc = poll( main_poll, main_poll_len, -1 );
		check( rc > 0, unlock_cleanup );

		for( i = 0; i < main_poll_len; i++ ) {

			if( main_poll[i].revents & main_poll[i].events ) {

				if( callbacks[i].req ) {

					callbacks[i].u.req_cb( callbacks[i].req, main_poll[i].revents );

				} else {

					callbacks[i].u.cb( main_poll[i].revents );

				}

			}

		}

#ifndef NO_PTHREADS
		rc = pthread_mutex_unlock( &main_poll_lock );
		check( rc == 0, final_cleanup );

		// TODO this doesn't work yet. Should force first in first out order using a mutex attribute
		// or a condition variable.
		// All the file descriptors adding and removing from other threads should take place here

		rc = pthread_mutex_lock( &main_poll_lock );
		check( rc == 0, final_cleanup );
#endif

	}

	return 0;

unlock_cleanup:
#ifndef NO_PTHREADS
	pthread_mutex_unlock( &main_poll_lock );

final_cleanup:
#endif
	return -1;

}
