#ifndef fra_p_lock_h
#define fra_p_lock_h

#ifdef NO_PTHREADS

#define fra_p_lock( L, E )

#else

#include <pthread.h>

int fra_p_pthreads;

#define fra_p_lock( L, E ) \
	if( fra_p_pthreads ) { \
		rc = pthread_mutex_lock( L ); \
		check( rc == 0, E ); \
	}
#endif

#ifdef NO_PTHREADS
#define fra_p_unlock( L, E )
#else
#define fra_p_unlock( L, E ) \
	if( fra_p_pthreads ) { \
		rc = pthread_mutex_unlock( ( L ) ); \
		check( rc == 0, E ); \
	}

#endif

#endif
