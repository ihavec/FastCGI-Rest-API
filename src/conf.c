#include "conf.h"




// private stuff

static int num_or_bools[ FRA_P_CONF_NUM_OR_BOOL_COUNT ];

static bstring strings[ FRA_P_CONF_STR_COUNT ];

struct str_arr {
	bstring v;
	int l;
};
static struct str_arr str_arrs[ FRA_P_CONF_STR_ARR_COUNT ];

struct conf {
	const char * name;
	enum fra_p_conf_type type;
	int id;
};

static struct conf [] = {
	{ "something", FRA_P_CONF_STRING, FRA_P_CONF_PLUGINS_DIR, "/default/value" },
	{ "check standard paths", FRA_P_CONF_BOOLEAN, FRA_P_CONF_CHECK_STD_PATHS, 0 },
	{ "enabled plugins", FRA_P_CONF_STRING_ARRAY, FRA_P_CONF_ENABLED_PLUGINS, "" }
};

// semi-private
int fra_p_conf_load( char * filename ) {

	// TODO
	// Using jsmn and jsonpath parse the config file that is passed to fra_glob_init( char * filename )
	// comments in json can be made as additional json objects:
	// example:
	// {
	// "#": " Should we also check the standard library paths ( /lib, /usr/lib, ... ) for the ",
	// "#": " *.so files of the enabled plugins ? Can be true or false.",
	// "check standard paths": true
	// }

}
