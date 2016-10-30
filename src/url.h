#ifndef fra_p_url_h
#define fra_p_url_h

#include "ht.h"
#include "end.h"



#pragma GCC visibility push(hidden)


int fra_p_url_init();

fra_p_ht_t * fra_p_url_ht_get();

fra_end_t * fra_p_url_to_endpoint( bstring verb, bstring url );


#pragma GCC visibility pop




#endif
