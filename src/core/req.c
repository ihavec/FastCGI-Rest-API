#include "req.h"
#include <fra/core.h>

#include "var.h"
#include "endpoint.h"
#include "dbg.h"
#include "config.h"
#include "hook.h"

#include <stdlib.h>
#include <poll.h>
#include <fcgiapp.h>
#ifndef NO_PTHREADS
#include <pthread.h>
#endif




// private functions and stuff ... :)

#ifndef NO_PTHREADS
static pthread_mutex_t empty_req_lock = PTHREAD_MUTEX_INITIALIZER;
#endif
static fra_req_t * empty_req;
static int empty_count;
static int all_count;
static int req_store_size;

static fra_req_t * get_req() {

	fra_req_t * cur;
#ifndef NO_PTHREADS
	int rc;


	rc = pthread_mutex_lock( &empty_req_lock );
	check( rc == 0, final_cleanup );
#endif

	if( ! empty_req ) {

		cur = malloc( sizeof( fra_req_t ) + req_store_size );
		check( cur, unlock_cleanup );

		cur->req_store = (void *)( (char *)cur + sizeof( fra_req_t ) );

		rc = fra_p_req_hook_execute( cur, FRA_REQ_CREATED );
		check( rc == 0, cur_cleanup );

		all_count++;

	} else {

		cur = empty_req;
		empty_req = cur->next;

	}

#ifndef NO_PTHREADS
	rc = pthread_mutex_unlock( &empty_req_lock );
	check( rc == 0, cur_cleanup );
#endif

	return cur;

cur_cleanup:
	free( cur );

unlock_cleanup:
#ifndef NO_PTHREADS
	pthread_mutex_unlock( &empty_req_lock );
final_cleanup:
#endif
	return NULL;

}

static int req_maybe_free( fra_req_t * req ) {

#ifndef NO_PTHREADS
	int rc;


	rc = pthread_mutex_lock( &empty_req_lock );
	check( rc == 0, final_cleanup );
#endif

	if( ( all_count - empty_count ) * FRA_CORE_WAITING_REQUESTS_GROWTH_FACTOR > all_count ) {

		rc = fra_p_req_hook_execute( req, FRA_REQ_FREE );
		check( rc == 0, unlock_cleanup );

		free( req );
		all_count--;

	} else {

		req->next = empty_req;
		empty_req = req;
		empty_count++;

	}

#ifndef NO_PTHREADS
	rc = pthread_mutex_unlock( &empty_req_lock );
	check( rc == 0, final_cleanup );
#endif

	return 0;

unlock_cleanup:
#ifndef NO_PTHREADS
	rc = pthread_mutex_unlock( &empty_req_lock );
	check( rc == 0, final_cleanup );

final_cleanup:
#endif
	return -1;

}

void reset_req( fra_req_t * req ) {

	req->fcgx_defined = 0;
	req->endpoint = NULL;

}




// semi-private functions

int fra_p_req_init() {

	int rc;


	rc = FCGX_Init();
	check( rc == 0, final_cleanup );

	empty_req = NULL;
	req_store_size = 0;

	return 0;

final_cleanup:
	return -1;

}

int fra_p_req_handle_new( short revents ) {

	int rc;
	fra_req_t * req;


	debug( "Handling new request" );

	check( revents & POLLIN, final_cleanup );

	rc = fra_p_glob_hook_execute( FRA_REQ_INCOMING );
	check( rc == 0, final_cleanup );

	req = get_req();
	check( req, final_cleanup );

	reset_req( req );

	rc = fra_p_req_hook_execute( req, FRA_REQ_NEW );
	check( rc == 0, req_cleanup );

	/* ... run all the hooks before fcgx has accepted anything ( for firewalls ... ) ... */

	FCGX_InitRequest( &req->fcgx, 0, 0 );

	rc = FCGX_Accept_r( &req->fcgx );
	check( rc >= 0, req_cleanup );

	req->fcgx_defined = 1;

	/* ... run all the hooks before endpoint is known ... */

	/* ... parse url to get endpoint ... */

	req->endpoint = NULL; // set to the real endpoint ...

	//req->endpoint_store = fra_p_endpoint_store_get( req->endpoint );

	FCGX_FPrintF(
			req->fcgx.out,
			"Status: 200 OK\r\n"
			"Content-type: application/json; charset=utf-8\r\n"
			"\r\n"
			"{s:0}"
			"\r\n"
		    );

	FCGX_Finish_r( &req->fcgx );

	rc = req_maybe_free( req );
	check( rc == 0, final_cleanup );

	return 0;

req_cleanup:
	rc = req_maybe_free( req );
	check( rc == 0, final_cleanup );

final_cleanup:
	return -1;

}




// public funcions

FCGX_Request * fra_req_fcgx( fra_req_t * req ) {
	return req->fcgx_defined ? &req->fcgx : NULL;
}

fra_endpoint_t * fra_req_endpoint( fra_req_t * req ) {
	return req->endpoint;
}
