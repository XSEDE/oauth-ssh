#include <string.h>

#include "auth_error.h"

#include "test/unit_test.h"

struct auth_error *
_auth_error_from_curl(struct _curl_error * ce)
{
	if (!ce) return NULL;

	struct auth_error * ae = calloc(sizeof(*ae), 1);
	ae->_ae = calloc(sizeof(*(ae->_ae)), 1);

	ae->_ae->source   = AUTH_ERR_SRC_CURL;
	ae->_ae->_u.curl  = ce;
	ae->error_message = ce->errmsg;

	return ae;
}

struct auth_error *
_auth_error_from_ep(struct _ep_errors * epe)
{
	if (!epe) return NULL;

	struct auth_error * ae = calloc(sizeof(*ae), 1);
	ae->_ae = calloc(sizeof(*(ae->_ae)), 1);

	ae->_ae->source   = AUTH_ERR_SRC_EP;
	ae->_ae->_u.ep    = epe;
	ae->error_message = epe->error_description;

	return ae;
}

/*
 * Free resources associated with the auth_error.
 */
void
_auth_error_free(struct auth_error * ae)
{
	if (ae)
	{
		ASSERT(ae->_ae);
		switch (ae->_ae->source)
		{
		case AUTH_ERR_SRC_CURL:
			_curl_error_free(ae->_ae->_u.curl);
			break;
		case AUTH_ERR_SRC_EP:
			_ep_errors_free(ae->_ae->_u.ep);
			break;
		default:
			ASSERT(0);
		}
		free(ae->_ae);
		free(ae);
	}
}
