#include <jak/dbg.h>
#include <bstrlib.h>
#include <stdlib.h>




#define FRA_STORE_NAME "req->store"
#define VARMAX_STEP 10




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
} ns_t;

static ns_t * * namespaces;




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
		( *nss )->varc = 0;
		( *nss )->varmax = VARMAX_STEP;

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
	bstring b1;
	bstring b2;
	int p1;
	int p2;
	struct tagbstring ns;
	int res;


	res = 0;

	b1 = bfromcstr( "FRA_NAMESPACE(" );
	check( b1, final_cleanup );

	p1 = binstrr( file, pos, b1 );

	if( p1 >= 0 ) {

		b2 = bfromcstr( ")" );
		check( b2 && p1 < pos, b1_cleanup );

		p2 = binstr( file, p1 + b1->slen, b2 );
		check( p2 >=0 && p2 > p1 + b1->slen, b2_cleanup );

		rc = trim( &ns, p1 + b1->slen, p2, file );
		check( rc == 0, b2_cleanup );

		if( biseq( &ns, namespace ) == 1 ) {
			res = 1;
		}

		bdestroy( b2 );

	}

	bdestroy( b1 );

	return res;

b2_cleanup:
	bdestroy( b2 );

b1_cleanup:
	bdestroy( b1 );

final_cleanup:
	return 0;

}

static int replace_var( bstring * f, int argc, bstring namespace, bstring name, bstring type, int ns_index, int var_index ) {

	int rc;
	int pos;
	int i;
	bstring r;

	for( i = 3; i < argc; i++ ) {
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
					debug_v( "Will replace variable \"%.*s\".", name->slen, name->data );
					r = bformat( "( (%s)( *( %s.data + %s.positions[%d][%d] ) ) )",
							type->data, FRA_STORE_NAME, FRA_STORE_NAME, ns_index, var_index  );
					check( r, final_cleanup );
					rc = breplace( f[i], pos, name->slen, r, 'z' );
					check( rc == BSTR_OK, final_cleanup );
					bdestroy( r );
				} else {
					debug_v( "Variable \"%.*s\" is not to be replaced...", name->slen, name->data );
				}
			}
		}
	}

	return 0;

final_cleanup:
	return -1;

}

int main( int argc, char * * argv ) {

	int rc;
	bstring * f;
	int i;
	FILE * file;
	bstring b1;
	bstring b2;
	bstring b3;
	int p1;
	int p2;
	int p3;
	int j;
	int k;
	int old_p2;
	ns_t * * nss;


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
	b3 = bfromcstr( ")" );
	check( b1 && b2 && b3, b_cleanup );

	for( i = 3; i < argc; i++ ) {
		p1 = 0;
		while( p1 >= 0 && p1 < f[i]->slen ) {
			if( ! p1 ) p1 = -1;
			p1 = binstr( f[i], p1 + 1, b1 );
			if( p1 >= 0 ) {

				struct tagbstring namespace;
				struct tagbstring tags[3];


				p3 = binstr( f[i], p1 + b1->slen, b3 );
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
			p1 = binstr( f[i], p1 + 1, b1 );
			if( p1 >= 0 ) {

				p3 = binstr( f[i], p1 + b1->slen, b3 );
				check( p3 > p1 && p3 <= f[i]->slen, b_cleanup );

				rc = bdelete( f[i], p1, p3 - p1 + 1 );
				check( rc == BSTR_OK, b_cleanup );

			}
		}
	}

	nss = namespaces;
	while( *nss ) {
		debug_v( "Namespace \"%s\":", ( *nss )->namespace->data );
		for( i = 0; i < ( *nss )->varc; i++ ) {
			debug_v( "    Variable \"%s\":", ( *nss )->varv[i].name->data );
			debug_v( "        Type: \"%s\"", ( *nss )->varv[i].init->data );
			debug_v( "        Initializer: \"%s\"", ( *nss )->varv[i].init->data );

			rc = replace_var(
					f,
					argc,
					( *nss )->namespace,
					( *nss )->varv[i].name,
					( *nss )->varv[i].type,
					nss - namespaces,
					i
					);
			check( rc == 0, b_cleanup )

		}
		nss++;
	}

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
	bdestroy( b3 );
	free( f );

	return 0;

b_cleanup:
	bdestroy( b1 );
	bdestroy( b2 );
	bdestroy( b3 );

f_cleanup:
	free( f );

final_cleanup:
	return -1;

usage_cleanup:
	debug( "Wrong usage. USAGE: ./fra_parse -o $OUTPUT_DIR $SRC1 $SRC2 ..." );
	return -1;

}
