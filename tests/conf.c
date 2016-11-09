#include "../src/conf.h"
#include "../src/dbg.h"



int main() {

	int rc;


	rc = fra_p_conf_init();
	check( rc == 0, final_cleanup );

	rc = fra_p_conf_load( "conf.json" );
	check( rc == 0, final_cleanup );

	fra_p_conf_deinit();

	return 0;

final_cleanup:
	return -1;

}
