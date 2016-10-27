#ifndef fra_core_helper_h
#define fra_core_helper_h



#include <stddef.h>




/**
 * Helper macros and functions used in core.h that shouldn't be used directly by the user.
 */

void * fra_var_get( fra_req_t * request, const char * name, int name_len, const char * type, int type_len );

int fra_end_var_reg( fra_end_t * endpoint, const char * name, const char * type, size_t size );

int fra_req_var_reg( const char * name, const char * type, size_t size );




#endif
