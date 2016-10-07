// plan how the library functions should look:

static int handle_endpoint1( fra_req_t * req ) {

	// this should better be done inside the library
	// This line must always be here so the user decides
	// which namespaces to use for which endpoints ...
	FRA_STORE( req->store, ns1 );

	// ...
	
	return 0;
}

static int handle_endpoint2( fra_req_t * req ) {

	//no good here should only be done once when creating req inside library
	FRA_STORE( req->store, second_enpoint_namespace );

	// ...

	return 0;
}

int main() {

	//...


	fra_reg( ns1, handle_endpoint1, "POST", "/price/%d", price_id );

	return 0;
}
