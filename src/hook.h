#ifndef fra_p_hook_h
#define fra_p_hook_h

#include <fra/core.h>




#pragma GCC visibility push(hidden)

typedef struct fra_p_hook fra_p_hook_t;
struct fra_p_hook {
	fra_p_hook_t * next;
	int (*callback)( fra_req_t * );
	float priority;
};

int fra_p_hook_init();

void fra_p_hook_deinit();

int fra_p_glob_hook_execute( enum fra_glob_hook_type type );

int fra_p_req_hook_execute( fra_req_t * req, enum fra_hook_type type );

int fra_p_endpoint_hook_execute( fra_req_t * req, enum fra_hook_type type );

#pragma GCC visibility pop




#endif
