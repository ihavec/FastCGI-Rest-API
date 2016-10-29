#ifndef fra_p_end_h
#define fra_p_end_h

#include "ht.h"
#include "hook.h"

#include <stddef.h>
#include <bstrlib.h>
#ifndef NO_PTHREADS
#include <pthread.h>
#endif




#pragma GCC visibility push(hidden)

struct fra_end {
	bstring name;
	size_t store_size;
	fra_p_ht_t * store_map;
	fra_p_hook_t * * hooks;
#ifndef NO_PTHREADS
	pthread_mutex_t lock;
#endif
};

int fra_p_end_init();

#pragma GCC visibility pop




#endif
