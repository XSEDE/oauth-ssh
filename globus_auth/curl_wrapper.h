#ifndef _GLOBUS_AUTH_CURL_WRAPPER_
#define _GLOBUS_AUTH_CURL_WRAPPER_

#include <curl/curl.h>

#include "curl_error.h"
#include "request.h"
#include "response.h"

/**********************************************************
 *
 * Debugging routines.
 *
 *********************************************************/
void
_curl_set_verbose(int debug); // print to stderr if debug

/**********************************************************
 *
 * Execute the request against the API endpoint.
 *
 *********************************************************/
struct _curl_error *
_curl_send_request(const char            * client_id, // NULL if bearer token used
                   const char            * secret_or_bearer_token,
                   const struct _request   request,
                   struct _response      * response);

/*
 * curl_easy_escape() but safe for HTML forms.
 * if InputLength == 0 then InputLength = strlen(InputValue)
 */
char * 
_curl_safe_escape(const char * InputValue, int InputLength);

#endif /* _GLOBUS_AUTH_CURL_WRAPPER_ */
