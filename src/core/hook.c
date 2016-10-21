#include <fra/core.h>




// private functions and stuff ... :)

struct hook {
	struct hook * next;
	int (*callback)( fra_req_t * );
	float priority;
};

#ifndef NO_PTHREADS
static pthread_mutex_t glob_hooks_lock = PHTREAD_MUTEX_INITIALIZER;
static pthread_mutex_t req_hooks_lock = PHTREAD_MUTEX_INITIALIZER;
#endif
static struct hook * * glob_hooks;
static struct hook * * req_hooks;




// semi-private functions

int fra_p_hook_init() {

	int rc;


	glob_hooks = calloc( FRA_GLOB_HOOK_COUNT, sizeof( struct hook * ) );
	check( glob_hooks, final_cleanup );

	req_hooks = calloc( FRA_HOOK_COUNT, sizeof( struct hook * ) );
	check( req_hooks, final_cleanup );

	return 0;

final_cleanup:
	return -1;

}



// public functions

int fra_glob_hook_register( enum fra_glob_hook_type, int (*callback)(), float priority ) {

	int rc;


