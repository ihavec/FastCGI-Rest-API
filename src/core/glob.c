#include <fra/core.h>

#include "dbg.h"
#include "poll.h"
#include "req.h"
#include "hook.h"




int fra_glob_init() {

	int rc;


	debug( "I'm alive :)" );

	rc = fra_p_poll_init();
	check( rc == 0, final_cleanup );

	rc = fra_p_req_init();
	check( rc == 0, final_cleanup );

	rc = fra_p_hook_init();
	check( rc == 0, final_cleanup );

	return 0;

final_cleanup:
	return -1;

}
