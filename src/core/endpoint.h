#ifndef fra_p_core_endpoint_h
#define fra_p_core_endpoint_h

#include "hashtable.h"

#include <stddef.h>
#include <bstrlib.h>




#pragma GCC visibility push(hidden)

struct fra_endpoint {
	bstring name;
	size_t store_size;
	fra_p_hashtable_t * store_map;
};

#pragma GCC visibility pop




#endif
