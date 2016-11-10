#include "conf.h"

#include "dbg.h"

#include <stdlib.h>
#include <jsmn.h>
#include <jsonpath.h>
#include <bstrlib.h>
#include <dirent.h>




// private stuff

enum fra_p_conf_type {
	FRA_P_CONF_STR,
	FRA_P_CONF_STR_ARR,
	FRA_P_CONF_NUMBER,
	FRA_P_CONF_BOOLEAN
};

static int num_or_bools[ FRA_P_CONF_NUM_OR_BOOL_COUNT ];

static bstring g_strings[ FRA_P_CONF_STR_COUNT ];

struct str_arr {
	bstring * v;
	int l;
};
static struct str_arr str_arrs[ FRA_P_CONF_STR_ARR_COUNT ];

struct conf {
	const char * name;
	enum fra_p_conf_type type;
	int id;
	const char * str;
	int val;
};

static struct conf confs[] = {
	{ "plugins directory", FRA_P_CONF_STR, FRA_P_CONF_PLUGINS_DIR, "/etc/fra/plugins", 0 },
	{ "thread safe", FRA_P_CONF_BOOLEAN, FRA_P_CONF_THREAD_SAFE, "", 0 },
	{ "enabled plugins", FRA_P_CONF_STR_ARR, FRA_P_CONF_ENABLED_PLUGINS, "", 0 }
};

static void str_arr_destroy( struct str_arr * a ) {

	int i;


	for( i = 0; i < a->l; i++ ) {

		bdestroy( a->v[i] );

	}

	free( a->v );
	a->v = NULL;
	a->l = 0;

}

static int set_defaults() {

	int rc;

	unsigned int i;


	for( i = 0; i < sizeof( confs ) / sizeof( struct conf ); i++ ) {

		if( confs[i].type == FRA_P_CONF_STR ) {

			if( confs[i].str ) {

				rc = bassigncstr( g_strings[ confs[i].id ], confs[i].str );

			} else {

				rc = bassigncstr( g_strings[ confs[i].id ], "" );

			}
			check( rc == BSTR_OK, final_cleanup );

		} else if( confs[i].type == FRA_P_CONF_NUMBER || confs[i].type == FRA_P_CONF_BOOLEAN ) {

			num_or_bools[ confs[i].id ] = confs[i].val;

		} else if( confs[i].type == FRA_P_CONF_STR_ARR ) {

			str_arr_destroy( &str_arrs[ confs[i].id  ] );

		} else {

			check( 0, final_cleanup );

		}

	}

	return 0;

final_cleanup:
	return -1;

}

static int set_value( struct conf * c, char * s, jsmntok_t * t, jsmntok_t * p ) {

	int rc;

	int val;


	if( c->type == FRA_P_CONF_STR ) {

		check_msg_v(
				t->type == JSMN_STRING,
				final_cleanup,
				"Error in configuration file. The value for the \"%.*s\" key has to be a string.",
				p->end - p->start,
				s + p->start
			   );

		rc = bassignblk( g_strings[ c->id ], s + t->start, t->end - t->start );
		check( rc == BSTR_OK, final_cleanup );

	} else if( c->type == FRA_P_CONF_BOOLEAN ) {

		check( t->type == JSMN_PRIMITIVE, not_a_boolean_cleanup );

		if( *( s + t->start ) == 't' ) {

			val = 1;

		} else if( *( s + t->start ) == 'f' ) {

			val = 0;

		} else {

			check( 0, not_a_boolean_cleanup );

		}

		num_or_bools[ c->id ] = val;

	} else if( c->type == FRA_P_CONF_NUMBER ){

		check( *( s + t->start ) >= '0' && *( s + t->start ) <= '9', not_a_number_cleanup );

		errno = 0;
		val = strtol( s + t->start, NULL, 10 );
		check( errno == 0, not_a_number_cleanup );

		num_or_bools[ c->id ] = val;

	} else {

		check( 0, final_cleanup );

	}

	return 0;

not_a_boolean_cleanup:
	log_err_v(
			"Error in configuration file. The value for the \"%.*s\" key has to be a boolean (true or false).",
			p->end - p->start,
			s + p->start
		 );
	return -1;


not_a_number_cleanup:
	log_err_v(
			"Error in configuration file. The value for the \"%.*s\" key has to be an integer number.",
			p->end - p->start,
			s + p->start
		 );
	return -1;

final_cleanup:
	return -1;

}




// semi-private

int fra_p_conf_init() {

	int i;


	for( i = 0; i < FRA_P_CONF_STR_COUNT; i++ ) {

		g_strings[i] = bfromcstr( "" );
		check( g_strings[i], final_cleanup );

	}

	for( i = 0; i < FRA_P_CONF_STR_ARR_COUNT; i++ ) {

		str_arrs[i].v = NULL;
		str_arrs[i].l = 0;

	}

	for( i = 0; i < FRA_P_CONF_NUM_OR_BOOL_COUNT; i++ ) num_or_bools[i] = 0;

	return 0;

final_cleanup:
	return -1;

}

void fra_p_conf_deinit() {

	int i;


	for( i = 0; i < FRA_P_CONF_STR_COUNT; i++ ) bdestroy( g_strings[i] );

}

int fra_p_conf_load( char * filename ) {

	int rc;

	jsmn_parser parser;
	jsmntok_t * t;
	size_t t_mlen;
	int t_len;
	jsmntok_t * tmp;
	FILE * f;
	bstring f_str;
	jjp_result_t * r;
	unsigned int i;
	struct tagbstring key;
	bstring new_filename;
	bstring dirname;
	unsigned int j;
	DIR * dir;
	struct dirent * dent;


	// TODO
	// Using jsmn and jsonpath parse the config file that is passed to fra_glob_init( char * filename )
	// comments in json can be made as additional json objects:
	// example:
	// {
	// "#": " Should we also check the standard library paths ( /lib, /usr/lib, ... ) for the ",
	// "#": " *.so files of the enabled plugins ? Can be true or false.",
	// "check standard paths": true
	// }

	rc = set_defaults();
	check( rc == 0, final_cleanup );

	f = fopen( filename, "r" );
	check_msg_v( f, final_cleanup, "No configuration file found at path \"%s\".", filename );

	f_str = bread( (bNread)fread, f );
	check( f_str, f_cleanup );

	t_mlen = 200;
	t = malloc( t_mlen * sizeof( jsmntok_t ) );
	check( t, f_str_cleanup );

	jsmn_init( &parser );

	t_len = jsmn_parse( &parser, bdata( f_str ), blength( f_str ), t, t_mlen );

	if( t_len == JSMN_ERROR_NOMEM ) {

		t_mlen = 1200;
		tmp = realloc( t, t_mlen * sizeof( jsmntok_t ) );
		check( tmp, t_cleanup );

		t_len = jsmn_parse( &parser, bdata( f_str ), blength( f_str ), t, t_mlen );

	}
	check_msg_v( t_len > 0, t_cleanup, "Invalid json in configuration file \"%s\".", filename );

	r = jjp_jsonpath( bdata( f_str ), t, t_len, "$.*", 0 );
	check( r, t_cleanup );

	for( i = 0; i < r->count; i++ ) {

		key.mlen = -1;
		key.slen = t[ t[ r->tokens[i] ].parent ].end - t[ t[ r->tokens[i] ].parent ].start;
		key.data = f_str->data + t[ t[ r->tokens[i] ].parent ].start;

		if( biseqStatic( &key, "include" ) == 1 ) {

			check_msg( t[ r->tokens[i] ].type == JSMN_STRING, r_cleanup,
					"Error in configuration file. The value for the \"include\" key has to be a string." );

			new_filename = blk2bstr( bdata( f_str ) + t[ r->tokens[i] ].start, t[ r->tokens[i] ].end - t[ r->tokens[i] ].start );
			check( new_filename, r_cleanup );

			rc = fra_p_conf_load( bdata( new_filename ) );
			bdestroy( new_filename );
			check( rc == 0, r_cleanup );

		} else if( biseqStatic( &key, "include directory" ) == 1 ) {

			check_msg( t[ r->tokens[i] ].type == JSMN_STRING, r_cleanup,
					"Error in configuration file. The value for the \"include dir\" key has to be a string." );

			dirname = blk2bstr( bdata( f_str ) + t[ r->tokens[i] ].start, t[ r->tokens[i] ].end - t[ r->tokens[i] ].start );
			check( dirname, r_cleanup );

			dir = opendir( (char *)dirname->data );
			check( dir, dirname_cleanup );

			while( ( dent = readdir( dir ) ) ) {

				if( dent->d_name[0] != '.' ) {

					new_filename = bformat( "%s/%s", bdata( dirname ), dent->d_name );
					check( new_filename, dir_cleanup );

					rc = fra_p_conf_load( bdata( new_filename ) );
					bdestroy( new_filename );
					check( rc == 0, dir_cleanup );

				}

			}

			rc = closedir( dir );
			check( rc == 0, dirname_cleanup );

			bdestroy( dirname );

		} else {

			for( j = 0; j < sizeof( confs ) / sizeof( struct conf ); j++ ) {

				if( biseqcstr( &key, confs[j].name ) == 1 ) {

					rc = set_value( &confs[j], bdata( f_str ), &t[ r->tokens[i] ], &t[ t[ r->tokens[i] ].parent ] );
					check( rc == 0, r_cleanup );

					break;

				}

			}

		}

	}

	jjp_result_destroy( r );
	free( t );
	bdestroy( f_str );

	rc = fclose( f );
	check( rc == 0, final_cleanup );


	return 0;

dir_cleanup:
	rc = closedir( dir );
	check( rc == 0, r_cleanup );

dirname_cleanup:
	bdestroy( dirname );

r_cleanup:
	jjp_result_destroy( r );

t_cleanup:
	free( t );

f_str_cleanup:
	bdestroy( f_str );

f_cleanup:
	rc = fclose( f );
	check( rc == 0, final_cleanup );

final_cleanup:
	return -1;

}

int fra_p_conf_num_or_bool( int id ) {

	return num_or_bools[id];

}

bstring fra_p_conf_str( int id ) {

	return g_strings[id];

}

int fra_p_conf_str_arr( int id, bstring * result, int result_len );
