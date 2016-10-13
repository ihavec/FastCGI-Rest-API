#ifndef fra_p_core_req_h
#define fra_p_core_req_h

#include <fra/core.h>




#pragma GCC visibility push(hidden)

struct fra_req {
	void * store;
	fra_endpoint_t * endpoint;
};

int fra_p_req_handle_new( short revents );

#pragma GCC visibility pop




#endif
