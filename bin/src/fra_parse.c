#include <jak/dbg.h>
#include <bstrlib.h>
#include <stdlib.h>




typedef struct {
	struct tagbstring name;
	struct tagbstring type;
	struct tagbstring init;
} var_t;

#define VARMAX_STEP 10
typedef struct {
	struct tagbstring namespace;
	var_t * varv;
	int varc;
	int varmax;
} ns_t;

static ns_t * * namespaces;




static void b_dup( bstring target, bstring original ) {
	target->slen = original->slen;
	target->mlen = original->mlen;
	target->data = original->data;
}

static inline int add_var( bstring namespace, bstring name, bstring type, bstring initializer ) {

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
		if( biseq( &( ( *nss )->namespace ), namespace ) == 1 ) {
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

		b_dup( &( ( *nss )->namespace ), namespace );
		( *nss )->varv = malloc( VARMAX_STEP * sizeof( var_t ) );
		( *nss )->varc = 0;
		( *nss )->varmax = VARMAX_STEP;

	}

	for( i = 0; i < ( *nss )->varc; i++ ) {
		if( biseq( &( ( *nss )->varv[i].name ), name ) == 1 ) {
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

	b_dup( &( ( *nss )->varv[ ( *nss )->varc ].name ), name );
	b_dup( &( ( *nss )->varv[ ( *nss )->varc ].type ), type );
	b_dup( &( ( *nss )->varv[ ( *nss )->varc ].init ), initializer );
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

int main( int argc, char * * argv ) {

	int rc;
	bstring * f;
	int i;
	FILE * file;
	bstring b1;
	bstring b2;
	bstring b3;
	bstring b4;
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
	b4 = bfromcstr( "FRA_NAMESPACE(" );
	check( b1 && b2 && b3 && b4, b_cleanup );

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

	nss = namespaces;
	while( *nss ) {
		debug_v( "Namespace \"%.*s\":", ( *nss )->namespace.slen, ( *nss )->namespace.data );
		for( i = 0; i < ( *nss )->varc; i++ ) {
			debug_v( "    Variable \"%.*s\":", ( *nss )->varv[i].name.slen, ( *nss )->varv[i].name.data );
			debug_v( "        Type: \"%.*s\"", ( *nss )->varv[i].init.slen, ( *nss )->varv[i].init.data );
			debug_v( "        Initializer: \"%.*s\"", ( *nss )->varv[i].init.slen, ( *nss )->varv[i].init.data );
		}
		nss++;
	}

	return 0;

b_cleanup:
	bdestroy( b1 );
	bdestroy( b2 );
	bdestroy( b3 );
	bdestroy( b4 );

f_cleanup:
	free( f );

final_cleanup:
	return -1;

usage_cleanup:
	debug( "Wrong usage. USAGE: ./fra_parse -o $OUTPUT_DIR $SRC1 $SRC2 ..." );
	return -1;

}
