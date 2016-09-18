FRA_VARIABLES( ns1, price, double, NULL );
FRA_NAMESPACE( ns1 );

static void helper( void * * my_store ) {
	price = 22.222222;
}

int test1( void * * my_store ) {

	helper( my_store );
	if( price == 22.222222 ) {
		return 0;
	} else {
		return -1;
	}

}
