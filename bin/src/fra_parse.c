#include <jak/dbg.h>
#include <bstrlib.h>
#include <stdlib.h>




static inline int add_var( bstring * f, int argc, int namespace_index, bstring namespace, int name_index, bstring name, bstring type, bstring initializer ) {

	return 0;

}

static inline char is_whitespace( char c ) {
	return 	(
			c == ' '
			|| c == '\n'
			|| c == '\t'
		);
}

static inline int trim( bstring s, int start, int end, bstring f ) {

	debug_v( "start:%d, end:%d, slen:%d", start, end, f->slen );
	check( start >= 0 && end <= f->slen, final_cleanup );

	while( is_whitespace( f->data[start] ) ) {
		start++;
		check( start < end, final_cleanup );
	}

	while( is_whitespace( f->data[end] ) ) {
		end--;
		check( start < end, final_cleanup );
	}

	s->data = f->data + start;
	s->slen = end - start;
	s->mlen = -1;

	debug_v( "string is: \"%*.s\"", s->slen, s->data );

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
		p1 = binstr( f[i], 0, b1 );
		if( p1 >= 0 ) {

			struct tagbstring namespace;
			struct tagbstring tags[3];


			p3 = binstr( f[i], p1, b3 );
			check( p3 >= 0, b_cleanup );


			p2 = binchr( f[i], p1, b2 );
			check( p2 >=0 && p2 < p3, b_cleanup );

			rc = trim( &namespace, p1, p2, f[i] );
			check( rc == 0, b_cleanup );


			k = 0;
			j = 0;
			old_p2 = p2;

			while( ( p2 = binchr( f[i], p2 + 1, b2 ) ) >= 0 && p2 <= p3 ) {

				old_p2 = p2;

				rc = trim( &tags[j], old_p2, p2, f[i] );
				check( rc == 0, b_cleanup );

				j++;

				if( j > 2 ) {

					j = 0;
					add_var( f, argc, i - 3, &namespace, k, &tags[0], &tags[1], &tags[2] );
					k++;

				}

			}
			debug_v( "k:%d, j:%d, p2:%d, p3:%d", k, j, p2, p3 );
			check( k > 0 && j == 0 && old_p2 == p3, b_cleanup );

		}
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
