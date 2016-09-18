#include "ns1.h"
#include "ns2and3.h"




int main() {

	int rc;
	void * * my_store;


	FRA_STORE( my_store, ns1, ns2, ns3 );

	rc = test1( my_store );
	rc += test2( my_store );

	if( rc == 0 ) {
		return 0;
	} else {
		return -1;
	}

}
