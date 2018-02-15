#ifndef _GLOBUS_AUTH_ERROR_H_
#define _GLOBUS_AUTH_ERROR_H_

#include "globus_auth.h"
#include "curl_wrapper.h"
#include "endpoint_errors.h"

/**************************************************************************
 *
 * Goals for our error design:
 *  - Provide a brief summary that can be displayed to end users without
 *    confusing them.
 *  - Provide complete context in logs for admin and dev investigation
 *************************************************************************/

typedef enum {AUTH_ERR_SRC_CURL, AUTH_ERR_SRC_EP} auth_error_src_t;

/*****************************************************************
 *
 * Our opaque struct to limit the dependency from other layers.
 *
 ****************************************************************/
struct _auth_error {
	auth_error_src_t source;
	union {
		struct _ep_errors  * ep;
		struct _curl_error * curl;
	} _u;
};

struct auth_error *
_auth_error_from_curl(struct _curl_error * ce);

struct auth_error *
_auth_error_from_ep(struct _ep_errors * epe);

/*
 * Free resources associated with the auth_error.
 */
void
_auth_error_free(struct auth_error * ae);

#endif /* _GLOBUS_AUTH_ERROR_H_ */
