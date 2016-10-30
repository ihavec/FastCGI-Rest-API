#ifndef fra_p_req_h
#define fra_p_req_h

#include <fra/core.h>

#include <fcgiapp.h>
#include <bstrlib.h>




#pragma GCC visibility push(hidden)


struct fra_req {
	char fcgx_defined;
	FCGX_Request fcgx;
	fra_req_t * next;
	void * req_store;
	fra_end_t * endpoint;
	void * endpoint_store;
	struct tagbstring url;
	struct tagbstring verb;
};

int fra_p_req_init();

int fra_p_req_handle_new( short revents );

#pragma GCC visibility pop




#endif
