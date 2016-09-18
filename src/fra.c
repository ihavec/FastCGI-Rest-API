int fra_reg( const char * method, const char * endpoint, void (*entry_callback)( void * ), ... ) {

	va_list argp;
	const char * name;


	va_start( argp, entry_callback );

	while( ( name = va_arg( argp, const char * ) ) ) {

	}

	fra_reg( "GET", "/buhu/skfjsdl", ksdjfslkd,
			FRA_VARIABLES(
				jskfj, int, NULL,
				lsdfjsdl,
				int alsdjds
				)
	       );
	return 0;
}

struct fra_ns0 {
	bstring var0;
	int var1;
	float var2;
};

//other namespaces (fra_store1, ...)

//...

int fra_req_init() {

	//FRA_STORE( req->store, ns1, ns2, ns3 );
	//where req->store must be of type void *
	//transforms to --->

	//where 4 is the count of namestores in FRA_STORE() + 1 call
	req->store = malloc( 4 * sizeof( void * ) );

	req->store[0] = malloc( sizeof( fra_store0 ) );
	req->store[1] = malloc( sizeof( fra_store1 ) );
	req->store[2] = malloc( sizeof( fra_store2 ) );
	req->store[3] = NULL;
	//so if people need to dealloc req->store they can just do while( *req->store) free( *req->store ); free( req->store );

}
