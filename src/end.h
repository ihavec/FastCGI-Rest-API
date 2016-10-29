#ifndef fra_p_core_end_h
#define fra_p_core_end_h

#include "var_ht.h"
#include "hook.h"

#include <stddef.h>
#include <bstrlib.h>




#pragma GCC visibility push(hidden)

struct fra_end {
	bstring name;
	size_t store_size;
	fra_p_var_ht_t * store_map;
	fra_p_hook_t * * hooks;
};

#pragma GCC visibility pop




#endif
