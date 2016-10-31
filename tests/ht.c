#include "../src/ht.h"
#include "../src/dbg.h"
#include <bstrlib.h>




struct test {
	bstring name;
	int pos;
	bstring url;
	long long something;
};

static int set_value( void * value, void * usr_arg ) {

	*( (struct test *)value ) = *( (struct test *)usr_arg );

	return 0;


}

static void destructor( void * v_v ) {

	struct test * v;


	v = (struct test *)v_v;
	bdestroy( v->name );
	bdestroy( v->url );

}

static int set_test( fra_p_ht_t * ht ) {

	int rc;

	struct test t;


	t.name = bfromcstr( "a name" );
	check( t.name, final_cleanup );

	t.pos = 12;

	t.something = LONG_MAX - 2000;

	t.url = bfromcstr( "http://somewhere.cc/" );
	check( t.url, final_cleanup );

	rc = fra_p_ht_set( ht, "/test/one", &t );
	check( rc == 0, final_cleanup );

	return 0;

final_cleanup:
	return -1;

}

static int set_test2( fra_p_ht_t * ht, int count ) {

	int rc;

	struct test t;
	bstring key;


	t.name = bformat( "a name %d", count );
	check( t.name, final_cleanup );

	t.pos = count;

	t.something = LONG_MAX - 20 * count;

	t.url = bfromcstr( "http://somewhere.cc/" );
	check( t.url, final_cleanup );

	key = bformat( "%d hbuhu", count );
	check( key, final_cleanup );

	rc = fra_p_ht_set( ht, bdata( key ), &t );
	check( rc == 0, final_cleanup );

	bdestroy( key );

	return 0;

final_cleanup:
	return -1;

}

int main() {

	int rc;

	fra_p_ht_t * ht;
	struct test * t;
	int i;


	ht = fra_p_ht_new( 15, sizeof( struct test ), set_value, destructor );
	check( ht, final_cleanup );

	rc = set_test( ht );
	check( rc == 0, final_cleanup );


	t = fra_p_ht_get( ht, "/test/one", strlen( "/test/one" ) );

	check(
			t
			&& t->pos == 12
			&& biseqcstr( t->name, "a name" ) == 1
			&& biseqcstr( t->url, "http://somewhere.cc/" ) == 1
			&& t->something == LONG_MAX - 2000,
			final_cleanup
	     );

	for( i = 0; i < 50; i++ ) {

		rc = set_test2( ht, i );
		check( rc == 0, final_cleanup );

	}

	fra_p_ht_free( ht );

	return 0;

final_cleanup:
	return -1;

}
