#ifndef fra_p_end_h
#define fra_p_end_h

#include "ht.h"
#include "hook.h"
#include "req.h"

#include <stddef.h>
#include <bstrlib.h>
#ifndef NO_PTHREADS
#include <pthread.h>
#endif




#pragma GCC visibility push(hidden)


struct  fra_p_end_store {
	struct fra_p_end_store * next;
	void * store;
};

struct fra_end {
	bstring name;
	size_t store_size;
	fra_p_ht_t * store_map;
	fra_p_end_store_t * store_empty;
	int store_empty_count;
	int store_all_count;
	fra_p_hook_t * * hooks;
#ifndef NO_PTHREADS
	pthread_mutex_t lock;
#endif
	int (*callback)( fra_req_t * );
};

int fra_p_end_store_set( fra_req_t * );

int fra_p_end_store_maybe_free( fra_req_t * r );

#pragma GCC visibility pop




#endif
