#ifndef fra_p_conf_h
#define fra_p_conf_h

#include <bstrlib.h>




#pragma GCC visibility push(hidden)


enum fra_p_conf_num_or_bool {
	FRA_P_CONF_CHECK_STD_PATHS;
	FRA_P_CONF_NUM_OR__BOOL_COUNT;
};

enum fra_p_conf_str {
	FRA_P_CONF_PLUGINS_DIR;
	FRA_P_CONF_CONF_D_DIR;
	FRA_P_CONF_STR_COUNT;
};

enum fra_p_conf_str_arr {
	FRA_P_CONF_ENABLED_PLUGINS;
	FRA_P_CONF_STR_ARR_COUNT;
};

int fra_p_conf_load( char * filename );

int fra_p_conf_num_or_bool( int id );

bstring fra_p_conf_str( int id );

int fra_p_conf_str_arr( int id, bstring * result, int result_len );


#pragma GCC visibility pop




#endif
