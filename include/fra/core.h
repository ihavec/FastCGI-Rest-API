#ifndef fra_core_h
#define fra_core_h


#include <fcgiapp.h>




/**
 * Main request type ( opaque/private )
 */
typedef struct fra_req fra_req_t;

/**
 * Main endpoint type ( opaque/private )
 */
typedef struct fra_end fra_end_t;

/**
 * Global init function. It is not thread safe! Should be called at least one time
 * before calling any other functions from the library.
 * \Returns 0 on success, 1 if it was already called previously, and -1 on error.
 */
int fra_glob_init();

/**
 * Global deinit function. It is not thread safe! Can be called at the end to cleanup.
 * Must be called exactly once.
 * You can call fra_glob_init() again afterwards if you need to use the library functions again.
 */
void fra_glob_deinit();

/**
 * Main macro for getting variables from inside a request.
 * Name should be a static string, because we operate on it using sizeof() for better performance.
 */
#define fra( request, name, type ) ( *( (type *)fra_var_get( request, name, sizeof( name ), #type, sizeof( #type ) ) ) )

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
#define fra_reg( endpoint, name, type ) fra_end_var_reg( endpoint, name, #type, sizeof( type ) )

/**
 * Macro for registering variables that are available in all requests.
 */
#define fra_req_reg( name, type ) fra_req_var_reg( name, #type, sizeof( type ) )

/**
 * Available hooks
 */
enum fra_glob_hook_type {
	FRA_REQ_INCOMING, /**< Called when a new request has come in but before any allocation of
			    the fra_req_t * object or processing is done. */
	FRA_GLOB_HOOK_COUNT
};

enum fra_hook_type {
	FRA_REQ_CREATED, /**< Called when a new fra_req_t is allocated to handle a request.
			   Use it to initialize variables,
			   register file descriptors ... */
	FRA_REQ_BEFORE_FCGX, /**< Called before the fastcgi library has done anything (parsed url, headers ...).
			       Use it for whatever... :) */
	FRA_REQ_BEFORE_ENDPOINT, /**< Called before any url parsing is done.
				   Use it for custom url parsing with the help of fra_req_endpoint_set()
				   */
	FRA_REQ_NEW, /**< Called when a new request comes in.
		       Use it to reset the variables if you need,
		       or for authentication, and for handling the actual request ... */
	FRA_REQ_FREE, /**< Called right before the fra_req_t is deallocated.
			Use it to free memory if you dynamically allocated some
			in FRA_REQ_CREATED. */
	FRA_HOOK_COUNT
};

/**
 * Register a new callback to be called at a specific part of the library.
 * Normal authentication hooks use priority 10.0f for the FRA_REQ_NEW hook_type.
 * Higher priority means later execution.
 * If the return value of the callback differs from 0 then the processing of the request is stopped and an error handler is called.
 * The default error handler sets the body to empty and the HTTP code of the response to the value returned from the callback
 * if it is greater than 0 or to 500 otherwise.
 */
int fra_glob_hook_reg(
		enum fra_glob_hook_type type,
		int (*callback)(),
		float priority
		);

int fra_req_hook_reg(
		enum fra_hook_type type,
		int (*callback)( fra_req_t * req ),
		float priority
		);

int fra_end_hook_reg(
		fra_end_t * endpoint,
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
 * This function returns only if you call fra_glob_poll_stop().
 * Else it polls the file descriptors forever.
 * If there is an error it also returns with a non-zero value.
 */
int fra_glob_poll();

/**
 * Stops the fra_glob_poll() functions. This can be used to stop the server.
 */
int fra_glob_poll_stop();

/**
 * Create an new empty endpoint that can be used to register variables, add matching urls ...
 */
fra_end_t * fra_end_new( int var_count );

/**
 * Free all memory allocated for an endpoint. endpoint arg can be NULL.
 * The object is still freed even when return code is -1 !!!
 * \Returns 0 on success and -1 if destroying the mutex lock fails.
 */
int fra_end_free( fra_end_t * endpoint );

/**
 * Add absolute url that should match this endpoint.
 * Multiple urls can match the same endpoint.
 */
int fra_end_url_add( fra_end_t * e, char * verb, char * url );

/**
 * Remove absolute url that should match this endpoint.
 */
int fra_end_url_del( fra_end_t * e, char * verb, char * url );

/**
 * Sets the callback to handle the requests sent to this endpoint
 */
int fra_end_callback_set( fra_end_t * e, int (*callback)( fra_req_t * ) );

//TODO
//add support for fra_end_urlformat_add( char * format, ... ) / del( char * format )
//for urls of kind ...add( "/product/%d", "product_id" ) ( available afterwards from fra_i( req, "product_id" ) )
//or ...add( "/product/%.*s", "product_name_len", "product_name" ) ( note that product_name is NOT null terminated!!! )

/**
 * Get FCGX_Request object for this request.
 */
FCGX_Request * fra_req_fcgx( fra_req_t * req );

/**
 * Get the endpoint for this request.
 */
fra_end_t * fra_req_endpoint( fra_req_t * r );

/**
 * Sets the endpoint for this request.
 * Useful for custom url parsing. From FRA_REQ_BEFORE_ENDPOINT hook for example.
 */
int fra_req_endpoint_set( fra_req_t * r, fra_end_t * e );




#endif
