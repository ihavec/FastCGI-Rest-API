#ifndef fra_p_pl_h
#define fra_p_pl_h

#include <bstrlib.h>




#pragma GCC visibility push(hidden)


typedef struct fra_p_pl {
	struct fra_p_pl * next;
	bstring name;
	char * * argv;
	int argc;
	void * h;
} fra_p_pl_t;


void fra_p_pl_free( fra_p_pl_t * pl );

void fra_p_pl_reset();

void fra_p_pl_add( fra_p_pl_t * pl );

int fra_p_pl_load();

int fra_p_pl_unload();


#pragma GCC visibility pop




#endif
