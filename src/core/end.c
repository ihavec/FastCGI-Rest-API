#include "end.h"
#include <fra/core.h>

#include "var.h"
#include "var_ht.h"
#include "dbg.h"
#include "hook.h"

#include <stdlib.h>




// public functions

fra_end_t * fra_end_new( int var_count ) {

	fra_end_t * e;


	e = malloc( sizeof( fra_end_t ) );
	check( e, final_cleanup );

	e->store_map = fra_p_var_ht_new( var_count );
	check( e->store_map, store_map_cleanup );

	e->hooks = calloc( FRA_GLOB_HOOK_COUNT, sizeof( fra_p_hook_t * ) );
	check( e->hooks, e_cleanup );

	return e;

store_map_cleanup:
	e->hooks = NULL;

e_cleanup:
	fra_end_free( e );

final_cleanup:
	return NULL;

}

void fra_end_free( fra_end_t * e ) {

	if( e ) {

		fra_p_var_ht_free( e->store_map );
		free( e->hooks );
		free( e );

	}

}
