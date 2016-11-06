#include <fra/core.h>

#include "dbg.h"
#include "poll.h"
#include "req.h"
#include "hook.h"
#include "var.h"
#include "url.h"




int fra_glob_init() {

	int rc;


	debug( "I'm alive :)" );

	rc = fra_p_poll_init();
	check( rc == 0, final_cleanup );

	rc = fra_p_hook_init();
	check( rc == 0, final_cleanup );

	rc = fra_p_var_init( 400 );
	check( rc == 0, final_cleanup );

	rc = fra_p_req_init();
	check( rc == 0, final_cleanup );

	rc = fra_p_url_init();
	check( rc == 0, final_cleanup );

	return 0;

final_cleanup:
	return -1;

}

void fra_glob_deinit() {

	fra_p_url_deinit();
	fra_p_req_deinit();
	fra_p_var_deinit();
	fra_p_hook_deinit();
	fra_p_poll_deinit();

}
