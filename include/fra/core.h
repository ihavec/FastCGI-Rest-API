#ifndef fra_core_h
#define fra_core_h




#include <fra/core_helper.h>




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
 * Main macro for registering variables for an endpoint,
 * that can later be used in any request thrown at that exact endpoint.
 */
#define fra_reg( endpoint, name, type ) fra_register( endpoint, name, #type, sizeof( type ) )

/**
 * Available hooks
 */
enum fra_hook_type {
	FRA_REQ_CREATE, /**< Called when a new fra_req_t is allocated to handle a request.
			  Use it to initialize variables */
	FRA_REQ_NEW /**< Called when a new request comes in for the specified endpoint.
		      Use it to reset the variables if you need,
		      or for authentication, ... */
};

/**
 * Register a new callback to be called at a specific part of the library.
 * Normal authentication hooks use priority 10.0f.
 * Higher priority means later execution.
 */
int fra_endpoint_hook_register(
		fra_endpoint_t * endpoint,
		enum fra_hook_type type,
		int (*callback)( fra_req_t * ),
		float priority
		);




#endif
