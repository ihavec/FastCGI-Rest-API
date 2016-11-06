#include "req.h"
#include <fra/core.h>

#include "var.h"
#include "end.h"
#include "dbg.h"
#include "config.h"
#include "hook.h"
#include "url.h"
#include "lock.h"

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

static fra_req_t * get_req() {

	int rc;

	fra_req_t * cur;
	size_t store_size;


	fra_p_lock( &empty_req_lock, final_cleanup );

	if( ! empty_req ) {

		store_size = fra_p_var_store_size_get( &rc );
		check( rc == 0, final_cleanup );

		cur = malloc( sizeof( fra_req_t ) + store_size );
		check( cur, unlock_cleanup );

		cur->req_store = (void *)( (char *)cur + sizeof( fra_req_t ) );

		rc = fra_p_req_hook_execute( cur, FRA_REQ_CREATED );
		check( rc == 0, cur_cleanup );

		all_count++;

	} else {

		cur = empty_req;
		empty_req = cur->next;
		empty_count--;

	}

	fra_p_unlock( &empty_req_lock, cur_cleanup );

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

	int rc;


	fra_p_lock( &empty_req_lock, final_cleanup );

	rc = fra_p_end_store_maybe_free( req );
	check( rc == 0, final_cleanup );

	debug_v( "all_count is %d and empty_count is %d, growth factor is %f", all_count, empty_count, FRA_CORE_WAITING_REQUESTS_GROWTH_FACTOR );

	if( all_count - empty_count > all_count * FRA_CORE_WAITING_REQUESTS_GROWTH_FACTOR ) {

		debug( "Freeing request." );

		rc = fra_p_req_hook_execute( req, FRA_REQ_FREE );
		check( rc == 0, unlock_cleanup );

		free( req );
		all_count--;

	} else {

		debug( "Caching request for later use." );

		req->next = empty_req;
		empty_req = req;
		empty_count++;

	}

	fra_p_unlock( &empty_req_lock, final_cleanup );

	return 0;

unlock_cleanup:
	fra_p_unlock( &empty_req_lock, final_cleanup );

final_cleanup:
	return -1;

}

void reset_req( fra_req_t * req ) {

	req->fcgx_defined = 0;
	req->endpoint = NULL;

}

static int set_param( fra_req_t * r, const char * key, int mlen, bstring target ) {

	char * param;
	int slen;


	check( target, final_cleanup );

	param = FCGX_GetParam( key, r->fcgx.envp );
	check( param, final_cleanup );

	slen = strnlen( param, mlen );
	check( slen < mlen && slen >= 0, final_cleanup );

	target->mlen = -1;
	target->slen = slen;
	target->data = (unsigned char *)param;

	return 0;

final_cleanup:
	return -1;

}

static int set_url( fra_req_t * r ) {

	char * param;
	int i;


	param = FCGX_GetParam( "REQUEST_URI", r->fcgx.envp );
	check( param, final_cleanup );

	r->base_url.data = (unsigned char *)param;
	r->base_url.slen = -1;
	r->base_url.mlen = -1;

	r->url.data = (unsigned char *)param;
	r->url.slen = -1;
	r->url.mlen = -1;

	r->query_url.data = (unsigned char *)"";
	r->query_url.slen = -1;
	r->query_url.mlen = -1;

	for( i = 0; param[i] != '\0'; i++ ) {

		check( i < FRA_CORE_MAX_URL_LENGTH, final_cleanup );

		if( param[i] == '?' && r->base_url.slen == -1 ) {

			r->base_url.slen = i;

			r->query_url.data = (unsigned char *)param + i;

		}

		// TODO parse get params to a hashtable of bstrings. Should use a safer function than murmurhash
		// or better just abort on too many params in one bucket as someone is trying the DOS attack on
		//  hashtables...
		//  maybe only parse when the user first requests a parameter? Probably smarter, so they can have
		//  their own implementations of parsing, that also escapes utf8 chars, or aborts on weird data ...

	}

	r->url.slen = i;

	if( r->base_url.slen == -1 ) {

		r->base_url.slen = i;

	}

	r->query_url.slen = r->url.slen - r->base_url.slen;

	return 0;

final_cleanup:
	return -1;

}




// semi-private functions

int fra_p_req_init() {

	int rc;


	rc = FCGX_Init();
	check( rc == 0, final_cleanup );

	empty_req = NULL;

	return 0;

final_cleanup:
	return -1;

}

void fra_p_req_deinit() {

	fra_req_t * cur;


	// Memory leak here as I didn't find any FCGX_Deinit() function
	// Added a valgrind suppression to ignore it in the tests ...

	while( empty_req ) {

		cur = empty_req;

		empty_req = empty_req->next;

		cur->endpoint = NULL;

		fra_p_req_hook_execute( cur, FRA_REQ_FREE );

		free( cur );

	}

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

	rc = fra_p_req_hook_execute( req, FRA_REQ_BEFORE_FCGX );
	check( rc == 0, req_cleanup );

	FCGX_InitRequest( &req->fcgx, 0, 0 );

	rc = FCGX_Accept_r( &req->fcgx );
	check( rc >= 0, fcgx_cleanup );

	req->fcgx_defined = 1;

	rc = set_param( req, "REQUEST_METHOD", FRA_CORE_MAX_VERB_LENGTH, &req->verb );
	check( rc == 0, fcgx_cleanup );

	rc = set_url( req );
	check( rc == 0, fcgx_cleanup );

	rc = fra_p_req_hook_execute( req, FRA_REQ_BEFORE_ENDPOINT );
	check( rc == 0, fcgx_cleanup );

	if( ! req->endpoint ) {

		req->endpoint = fra_p_url_to_endpoint( &req->verb, &req->base_url );
		check( req->endpoint, fcgx_cleanup );

	}

	// TODO have the same system than for req_empty for end_store_empty for each endpoint
	rc = fra_p_end_store_set( req );
	check( rc == 0, fcgx_cleanup );

	rc = fra_p_req_hook_execute( req, FRA_REQ_NEW );
	check( rc == 0, fcgx_cleanup );

	if( req->endpoint->callback ) req->endpoint->callback( req );

	/*
	FCGX_FPrintF(
			req->fcgx.out,
			"Status: 200 OK\r\n"
			"Content-type: application/json; charset=utf-8\r\n"
			"\r\n"
			"{s:0}"
			"\r\n"
		    );
		    */

	FCGX_Finish_r( &req->fcgx );

	rc = req_maybe_free( req );
	check( rc == 0, final_cleanup );

	return 0;

fcgx_cleanup:
	FCGX_FPrintF(
			req->fcgx.out,
			"Status: 500 Internal Server Error\r\n"
			"Content-type: application/json; charset=utf-8\r\n"
			"\r\n"
			"{ \"s\":-1, \"error\": \"Go away\" }"
			"\r\n"
		    );
	FCGX_Finish_r( &req->fcgx );

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

fra_end_t * fra_req_endpoint( fra_req_t * req ) {
	return req->endpoint;
}
