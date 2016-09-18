#include <jak/dbg.h>
#include <bstrlib.h>
#include <stdlib.h>




#define VARMAX_STEP 10
#define GEN_BEGIN "\n//FRAPARSE AUTO GENERATED START\n"
#define GEN_END "\n//FRAPARSE AUTO GENERATED END\n"




typedef struct {
	bstring name;
	bstring type;
	bstring init;
} var_t;

typedef struct {
	bstring namespace;
	var_t * varv;
	int varc;
	int varmax;
	char * declared_in_f;
	bstring store;
	int store_index;
	int store_id;
	int store_f;
} ns_t;

static ns_t * * namespaces;
static bstring * f;
static int g_argc;
static bstring b_fra_name; // a "FRA_NAMESPACE(" bstring
static bstring b_include; // a "#include" bstring




static int add_var( bstring namespace, bstring name, bstring type, bstring initializer ) {

	ns_t * * nss;
	int namespacesc;
	ns_t * * tmp;
	var_t * tmp_var;
	int i;


	if( ! namespaces ) {
		namespaces = malloc( sizeof( ns_t * ) );
		*namespaces = NULL;
	}

	nss = namespaces;
	while( *nss ) {
		if( biseq( ( *nss )->namespace, namespace ) == 1 ) {
			break;
		}
		nss++;
	}

	if( ! *nss ) {

		namespacesc = nss - namespaces;
		tmp = realloc( namespaces, ( namespacesc + 1 ) * sizeof( ns_t * ) );
		check( tmp, final_cleanup );

		namespaces = tmp;
		nss = namespaces + namespacesc;
		*( nss + 1) = NULL;

		*nss = malloc( sizeof( ns_t ) );
		check( *nss, final_cleanup );

		( *nss )->namespace = bstrcpy( namespace );
		check( ( *nss )->namespace, final_cleanup );
		( *nss )->varv = malloc( VARMAX_STEP * sizeof( var_t ) );
		check( (*nss)->varv, final_cleanup);
		( *nss )->varc = 0;
		( *nss )->varmax = VARMAX_STEP;
		( *nss )->declared_in_f = calloc( g_argc, sizeof( int ) );
		check( ( *nss  )->declared_in_f, final_cleanup );
		( *nss )->store = NULL;
		( *nss )->store_index = -1;
		( *nss )->store_id = -1;
		( *nss )->store_f = -1;

	}

	for( i = 0; i < ( *nss )->varc; i++ ) {
		if( biseq( ( *nss )->varv[i].name, name ) == 1 ) {
			debug_v( "Duplicate variable \"%.*s\" in namespace \"%.*s\". Aborting...",
					name->slen, name->data, namespace->slen, namespace->data );
			check( 0, final_cleanup );
		}
	}

	if( ( *nss )->varc == ( *nss )->varmax ) {
		tmp_var = realloc( ( *nss )->varv, ( ( *nss )->varmax + VARMAX_STEP ) * sizeof( var_t ) );
		check( tmp_var, final_cleanup );

		( *nss )->varv = tmp_var;
	}

	( *nss )->varv[ ( *nss )->varc ].name = bstrcpy( name );
	check( ( *nss )->varv[ ( *nss )->varc ].name, final_cleanup );

	( *nss )->varv[ ( *nss )->varc ].type = bstrcpy( type );
	check( ( *nss )->varv[ ( *nss )->varc ].type, final_cleanup );

	( *nss )->varv[ ( *nss )->varc ].init = bstrcpy( initializer );
	check( ( *nss )->varv[ ( *nss )->varc ].init, final_cleanup );

	( *nss )->varc++;

	return 0;

final_cleanup:
	return -1;

}

static inline char is_whitespace( char c ) {
	return 	(
			c == ' '
			|| c == '\n'
			|| c == '\t'
		);
}

static inline int trim( bstring s, int start, int end, bstring f ) {

	check( start >= 0 && end <= f->slen, final_cleanup );

	while( is_whitespace( f->data[start] ) ) {
		start++;
		check( start < end, final_cleanup );
	}

	while( is_whitespace( f->data[end - 1] ) ) {
		end--;
		check( start < end, final_cleanup );
	}

	s->data = f->data + start;
	s->slen = end - start;
	s->mlen = -1;

	return 0;

final_cleanup:
	return -1;

}

static inline char is_delimiter( char c ) {
	if(
			c == ';'
			|| c == ','
			|| c == '('
			|| c == ')'
			|| c == '{'
			|| c == '}'
			|| c == '['
			|| c == ']'
			|| is_whitespace( c )
	  ) {
		return 1;
	} else {
		return 0;
	}
}

static char is_from_namespace( bstring namespace, bstring file, int pos ) {

	int rc;
	int p1;
	int p2;
	struct tagbstring ns;
	int res;


	res = 0;

	p1 = binstrr( file, pos, b_fra_name );

	if( p1 >= 0 ) {

		check( p1 < pos, final_cleanup );

		p2 = bstrchrp( file, ')', p1 + b_fra_name->slen );
		check( p2 >=0 && p2 > p1 + b_fra_name->slen, final_cleanup );

		rc = trim( &ns, p1 + b_fra_name->slen, p2, file );
		check( rc == 0, final_cleanup );

		if( biseq( &ns, namespace ) == 1 ) {
			res = 1;
		}

	}

	return res;

final_cleanup:
	return 0;

}

static int define_namespace_in_file( int ns_index, bstring bf ) {

	int rc;
	int p;
	int i;
	bstring b;


	p = binstrr( bf, bf->slen - 1, b_include );
	if( p < 0 ) {
		p = 0;
	} else {
		p = bstrchrp( bf, '\n', p + 1 ) + 1;
		check( p - 1 > 0, final_cleanup );
	}

	b = bformat( GEN_BEGIN "struct fra_ns%d {\n", ns_index );
	check( b, final_cleanup );

	for( i = 0; i < namespaces[ ns_index ]->varc; i++ ) {

		rc = bformata( b, "%s var%d;\n", bdata( namespaces[ ns_index ]->varv[i].type ), i);
		check( rc == BSTR_OK, b_cleanup );

	}

	rc = bcatcstr( b, "};" GEN_END );
	check( rc == BSTR_OK, b_cleanup );

	rc = binsert( bf, p, b, ' ' );
	check( rc == BSTR_OK, b_cleanup );

	bdestroy( b );

	return 0;

b_cleanup:
	bdestroy( b );

final_cleanup:
	return -1;

}

static int replace_var( int ns_index, int var_index ) {

	int rc;
	int pos;
	int i;
	bstring r;
	bstring name;
	bstring namespace;


	namespace = namespaces[ ns_index ]->namespace;
	name = namespaces[ ns_index ]->varv[ var_index ].name;

	for( i = 3; i < g_argc; i++ ) {
		pos = 0;
		while( pos >= 0 && pos < f[i]->slen ) {
			if( ! pos ) pos = -1;
			pos = binstr( f[i], pos + 1, name );
			if( pos >= 0 ) {
				if(
						(
						 pos == 0
						 || is_delimiter( *( f[i]->data + pos - 1 ) )
						)
						&&
						(
						 pos + name->slen == f[i]->slen
						 || is_delimiter( *( f[i]->data + pos + name->slen ) )
						)
						&& is_from_namespace( namespace, f[i], pos )
				  ) {

					if( ! namespaces[ ns_index ]->declared_in_f[i] ) {
						namespaces[ ns_index ]->declared_in_f[i] = 1;
					}

					debug_v( "Will replace variable \"%.*s\".", name->slen, name->data );

					check( namespaces[ ns_index ]->store, msg_cleanup );

					r = bformat(
							"(  ( (struct fra_ns%d)( %s[%d] ) ).var%d )",
							ns_index,
							bdata( namespaces[ ns_index ]->store ),
							namespaces[ ns_index ]->store_index,
							var_index
							);
					check( r, final_cleanup );

					rc = breplace( f[i], pos, name->slen, r, ' ' );
					check( rc == BSTR_OK, final_cleanup );

					bdestroy( r );

				} else {
					debug_v( "Variable \"%.*s\" is not to be replaced...", name->slen, name->data );
				}
			}
		}
	}

	return 0;

msg_cleanup:
	debug_v( "No FRA_STORE() call found for namespace %s", bdata( namespace ) );
final_cleanup:
	return -1;

}

static int delete_all_occurrences( bstring find ) {

	int rc;
	int i;
	int p1;
	int p3;


	for( i = 3; i < g_argc; i++ ) {
		p1 = 0;
		while( p1 >= 0 && p1 < f[i]->slen ) {
			if( ! p1 ) p1 = -1;
			p1 = binstr( f[i], p1 + 1, find );
			if( p1 >= 0 ) {

				p3 = bstrchrp( f[i], ')', p1 + find->slen );
				check( p3 > p1 && p3 <= f[i]->slen, final_cleanup );

				rc = bdelete( f[i], p1, p3 - p1 + 1 );
				check( rc == BSTR_OK, final_cleanup );

			}
		}
	}

	return 0;

final_cleanup:
	return -1;

}

static int set_store_for_namespace( int file_id, int store_id, bstring store, bstring namespace ) {

	ns_t * * nss;
	ns_t * ns;
	int i;


	for( nss = namespaces, ns = NULL, i = 0; *nss; nss++ ) {

		if( biseq( namespace, ( *nss )->namespace ) == 1 ) {
			ns = *nss;
		}

		if( file_id == ( *nss )->store_f && store_id == ( *nss )->store_id  ) {
			i++;
		}
	}
	check( ns, msg_cleanup );

	ns->store = bstrcpy( store );
	check( ns->store, final_cleanup );

	ns->store_index = i;
	ns->store_f = file_id;
	ns->store_id = store_id;

	return 0;

msg_cleanup:
	debug_v( "Namespace named %.*s from FRA_STORE() call not found.", blength( namespace ), bdata( namespace ) );

final_cleanup:
	return -1;

}

static int replace_fra_store_code( int file_id, int store_id, bstring bf, int pos, int len ) {

	int rc;
	bstring r;
	ns_t * * nss;
	int c;
	bstring store;


	for( nss = namespaces, c = 0; *nss; nss++ ) {
		if(  ( *nss )->store_f == file_id && ( *nss )->store_id == store_id ) {
			c++;
			store = ( *nss )->store;
		}
	}
	check( c > 0, msg_cleanup );

	r = bformat( GEN_BEGIN "%s = malloc( %d * sizeof( void * ) );\n", bdata( store ), c + 1 );
	check( r, final_cleanup );

	for( nss = namespaces; *nss; nss++ ) {
		if(  ( *nss )->store_f == file_id && ( *nss )->store_id == store_id ) {
			rc = bformata(
					r,
					"%s[%d] = malloc( sizeof( struct fra_store%d ) );\n",
					bdata( store ),
					( *nss )->store_index,
					( *nss )->store_index
				     );
			check( rc == BSTR_OK && ( *nss )->store_index < c, r_cleanup );
		}
	}

	rc = bformata( r, "%s[%d] = NULL;" GEN_END, bdata( store ), c );
	check( rc == BSTR_OK, r_cleanup );

	rc = breplace( bf, pos, len, r, ' ' );
	check( rc == BSTR_OK, r_cleanup );

	bdestroy( r );

	return 0;

r_cleanup:
	bdestroy( r );

final_cleanup:
	return -1;

msg_cleanup:
	debug( "FRA_STORE() called without any namespaces." );
	return -1;

}

int main( int argc, char * * argv ) {

	int rc;
	int i;
	FILE * file;
	bstring b1;
	bstring b2;
	int p1;
	int p2;
	int p3;
	int j;
	int k;
	int old_p2;
	ns_t * * nss;
	bstring b_fra_store;


	check( argc > 3 && strcmp( argv[1], "-o" ) == 0, usage_cleanup );

	f = malloc( sizeof( bstring ) * argc );
	check( f, final_cleanup );

	for( i = 3; i < argc; i++) {

		file = fopen( argv[i], "r" );
		check( file, f_cleanup );

		f[i] = bread( (bNread)fread, file );
		fclose( file );
		check( f[i], f_cleanup );

	}

	b1 = bfromcstr( "FRA_VARIABLES(" );
	b2 = bfromcstr( ",)" );
	b_fra_name = bfromcstr( "FRA_NAMESPACE(" );
	b_include = bfromcstr( "#include" );
	b_fra_store = bfromcstr( "FRA_STORE(" );
	check( b1 && b2 && b_fra_name && b_include && b_fra_store, b_cleanup );

	g_argc = argc;

	for( i = 3; i < argc; i++ ) {
		p1 = 0;
		while( p1 >= 0 && p1 < f[i]->slen ) {
			if( ! p1 ) p1 = -1;
			p1 = binstr( f[i], p1 + 1, b1 );
			if( p1 >= 0 ) {

				struct tagbstring namespace;
				struct tagbstring tags[3];


				p3 = bstrchrp( f[i], ')', p1 + b1->slen );
				check( p3 >= 0, b_cleanup );


				p2 = binchr( f[i], p1 + b1->slen, b2 );
				check( p2 >=0 && p2 < p3, b_cleanup );

				rc = trim( &namespace, p1 + b1->slen, p2, f[i] );
				check( rc == 0, b_cleanup );


				k = 0;
				j = 0;
				old_p2 = p2;

				while( ( p2 = binchr( f[i], p2 + 1, b2 ) ) >= 0 && p2 <= p3 ) {

					rc = trim( &tags[j], old_p2 + 1, p2, f[i] );
					check( rc == 0, b_cleanup );

					old_p2 = p2;

					j++;

					if( j > 2 ) {

						j = 0;
						rc = add_var( &namespace, &tags[0], &tags[1], &tags[2] );
						check( rc == 0, b_cleanup );
						k++;

					}

				}
				check( k > 0 && j == 0 && old_p2 == p3, b_cleanup );

			}
		}
	}

	for( i = 3; i < argc; i++ ) {

		p1 = 0;
		while( p1 >= 0 && p1 < f[i]->slen ) {
			if( ! p1 ) p1 = -1;
			p1 = binstr( f[i], p1 + 1, b_fra_store );
			if( p1 >= 0 ) {

				struct tagbstring store;
				struct tagbstring namespace;


				p3 = bstrchrp( f[i], ')', p1 + b_fra_store->slen );
				check( p3 >= 0, b_cleanup );

				p2 = binchr( f[i], p1 + b_fra_store->slen, b2 );
				check( p2 >=0 && p2 < p3, b_cleanup );

				rc = trim( &store, p1 + b_fra_store->slen, p2, f[i] );
				check( rc == 0, b_cleanup );

				old_p2 = p2;

				while( ( p2 = binchr( f[i], p2 + 1, b2 ) ) >= 0 && p2 <= p3 ) {

					rc = trim( &namespace, old_p2 + 1, p2, f[i] );
					check( rc == 0, b_cleanup );

					old_p2 = p2;

					rc = set_store_for_namespace( i, p1, &store, &namespace );
					check( rc == 0, b_cleanup );

				}
				check( old_p2 == p3, b_cleanup );

				rc = replace_fra_store_code( i, p1, f[i], p1, p3 - p1 + 1 );
				check( rc == 0, b_cleanup );

			}

		}
	}

	rc = delete_all_occurrences( b1 );
	check( rc == 0, b_cleanup );

	for( nss = namespaces; *nss; nss++ ) {
		debug_v( "Namespace \"%s\":", ( *nss )->namespace->data );
		for( i = 0; i < ( *nss )->varc; i++ ) {
			debug_v( "    Variable \"%s\":", ( *nss )->varv[i].name->data );
			debug_v( "        Type: \"%s\"", ( *nss )->varv[i].type->data );
			debug_v( "        Initializer: \"%s\"", ( *nss )->varv[i].init->data );

			rc = replace_var( nss - namespaces, i );
			check( rc == 0, b_cleanup )

		}
	}

	for( nss = namespaces; *nss; nss++ ) {
		for( i = 3; i < argc; i++ ) {
			if( ( *nss )->declared_in_f[i] == 1 ) {
				rc = define_namespace_in_file( nss - namespaces, f[i] );
				check( rc == 0, b_cleanup );
			}
		}
	}

	rc = delete_all_occurrences( b_fra_name );
	check( rc == 0, b_cleanup );

	for( i = 3; i < argc; i++ ) {
		rc = bassignformat( b1, "%s/%s", argv[2], argv[i] );
		check( rc == BSTR_OK, b_cleanup );
		debug_v( "File path is: %s", bdata( b1 ) );
		file = fopen( bdata( b1 ), "w" );
		check( file, b_cleanup );
		rc = fwrite( f[i]->data, f[i]->slen, 1, file );
		fclose( file );
		check( rc == 1, b_cleanup );
	}

	//TODO free namespaces array, for now let the OS handle it :)
	bdestroy( b1 );
	bdestroy( b2 );
	bdestroy( b_fra_name );
	bdestroy( b_include );
	bdestroy( b_fra_store );
	free( f );

	return 0;

b_cleanup:
	bdestroy( b1 );
	bdestroy( b2 );
	bdestroy( b_fra_name );
	bdestroy( b_include );
	bdestroy( b_fra_store );

f_cleanup:
	free( f );

final_cleanup:
	return -1;

usage_cleanup:
	debug( "Wrong usage. USAGE: ./fra_parse -o $OUTPUT_DIR $SRC1 $SRC2 ..." );
	return -1;

}
