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
 * Main macro for registering variables to an endpoint,
 * Later they can be used in any request that has the specified endpoint.
 */
#define fra_reg( endpoint, name, type ) fra_register( endpoint, name, #type, sizeof( type ) )

/**
 * Available hooks
 */
enum fra_glob_hook_type {
	FRA_REQ_INCOMING /**< Called when a new request has come in but before any allocation of
			    the fra_req_t * object or processing is done */
};

enum fra_hook_type {
	FRA_REQ_CREATED, /**< Called when a new fra_req_t is allocated to handle a request.
			   Use it to initialize variables,
			   register file descriptors ... */
	FRA_REQ_NEW, /**< Called when a new request comes in.
		       Use it to reset the variables if you need,
		       or for authentication, and for handling the actual request ... */
};

/**
 * Register a new callback to be called at a specific part of the library.
 * Normal authentication hooks use priority 10.0f for the FRA_REQ_NEW hook_type.
 * Higher priority means later execution.
 * If the return value of the callback differs from 0 then the processing of the request is stopped and an error handler is called.
 * The default error handler sets the body to empty and the HTTP code of the response to the value returned from the callback
 * if it is greater than 0 or to 500 otherwise.
 */
int fra_glob_hook_register(
		enum fra_glob_hook_type,
		int (*callback)(),
		float priority
		);

int fra_req_hook_register(
		fra_req_t * request,
		enum fra_hook_type type,
		int (*callback)( fra_req_t * req ),
		float priority
		);

int fra_endpoint_hook_register(
		fra_endpoint_t * endpoint,
		enum fra_hook_type type,
		int (*callback)( fra_req_t * req ),
		float priority
		);


#include <fra/core_helper.h>


/**
 * File descriptors and polling functions:
 */

/**
 * Add a file descriptor to the poll that is connected to a fra_req_t *.
 */
int fra_req_fd_add(
		fra_req_t * req,
		int file_descriptor,
		short events,
		int (*callback)( fra_req_t * req, short revents ) );

/**
 * Add a global file descriptor to the poll.
 */
int fra_glob_fd_add(
		int file_descriptor,
		short events,
		int (*callback)( short revents ) );

/**
 * This function never returns. It polls the file descriptors forever. If there is an error it does return with a non-zero value.
 */
int fra_glob_poll();




#endif
