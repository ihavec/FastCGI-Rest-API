#include <fra/core.h>
#include "../src/dbg.h"
#include "../src/var.h"




int main() {

	int rc;

	fra_end_t * e;


	rc = fra_p_var_init( 100 );
	check( rc == 0, final_cleanup );

	e = fra_end_new( 100 );
	check( e, final_cleanup );

	fra_end_destroy( e );

	fra_p_var_deinit();

	return 0;

final_cleanup:
	return -1;

}
