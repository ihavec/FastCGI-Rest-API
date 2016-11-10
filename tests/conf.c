#include "../src/conf.h"
#include "../src/dbg.h"
#include <bstrlib.h>




int main() {

	int rc;


	rc = fra_p_conf_init();
	check( rc == 0, final_cleanup );

	rc = fra_p_conf_load( "conf.json" );
	check( rc == 0, final_cleanup );

	check( biseqStatic( fra_p_conf_str( FRA_P_CONF_PLUGINS_DIR ), "/buhu/behe/plugins" ) == 1, final_cleanup );

	check( fra_p_conf_num_or_bool( FRA_P_CONF_THREAD_SAFE ) == 0, final_cleanup );

	fra_p_conf_deinit();

	return 0;

final_cleanup:
	return -1;

}
