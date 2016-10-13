#ifndef fra_core_helper_h
#define fra_core_helper_h



#include <stddef.h>




/**
 * Helper macros and functions used in core.h that shouldn't be used directly by the user.
 */

void * fra_var_get( fra_req_t * request, char * name, const char * type );

int fra_req_register( fra_req_t * request, char * name, const char * type, size_t size );

int fra_endpoint_register( fra_endpoint_t * endpoint, char * name, const char * type, size_t size );




#endif
