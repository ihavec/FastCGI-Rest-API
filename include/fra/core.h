#ifndef fra_core_h
#define fra_core_h




/**
 * Main request type ( opaque/private )
 */
typedef struct fra_req fra_req_t;

/**
 * Main endpoint type ( opaque/private )
 */
typedef struct fra_endpoint fra_endpoint_t;

/**
 * Main macro for getting variables from inside a request.
 */
#define fra( request, name, type ) ( *( (type *)fra_var_get( request, name, #type ) ) )

/**
 * Utility macros for for getting variables of all native C types and some widely used pointers
 */
// native types
#define fra_c( request, name ) fra( request, name, char )
#define fra_sc( request, name ) fra( request, name, signed char )
#define fra_uc( request, name ) fra( request, name, unsigned char )
#define fra_s( request, name ) fra( request, name, short )
#define fra_us( request, name ) fra( request, name, unsigned short )
#define fra_i( request, name ) fra( request, name, int )
#define fra_ui( request, name ) fra( request, name, unsigned int )
#define fra_l( request, name ) fra( request, name, long )
#define fra_ul( request, name ) fra( request, name, unsigned long )
#define fra_ll( request, name ) fra( request, name, long long )
#define fra_ull( request, name ) fra( request, name, unsigned long long )
#define fra_f( request, name ) fra( request, name, float )
#define fra_d( request, name ) fra( request, name, double )
#define fra_ld( request, name ) fra( request, name, long double )
// widely used pointers
#define fra_cp( request, name ) fra( request, name, char * )

/**
 * Main macro for registering variables of an endpoint,
 * that can later be used in any request thrown at that exact enpoint.
 */
#define fra_reg( endpoint, name, type ) fra_register( endpoint, name, #type, sizeof( type ) )



// Helper functions for macros.

/**
 * Safer when used via the fra() macro then directly
 */
void * fra_var_get( fra_req_t * request, char * name, const char * type );

/**
 * Better used via the fra_req() macro then directly
 */
int fra_register( fra_endpoint_t * endpoint, char * name, const char * type, size_t size );




#endif
