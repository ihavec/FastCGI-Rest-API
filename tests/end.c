#include <fra/core.h>
#include "../src/dbg.h"




int main() {

	fra_end_t * e;


	e = fra_end_new( 100 );
	check( e, final_cleanup );

	fra_end_free( e );

	return 0;

final_cleanup:
	return -1;

}
