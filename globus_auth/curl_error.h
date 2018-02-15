#ifndef _GLOBUS_AUTH_CURL_ERROR_H
#define _GLOBUS_AUTH_CURL_ERROR_H

#include <curl/curl.h>

/**********************************************************
 *
 * Error handling routines.
 *
 *********************************************************/
struct _curl_error {
	CURLcode code;
	char     errmsg[CURL_ERROR_SIZE];
};

struct _curl_error *
_curl_error(CURLcode code, const char * errmsg, int msglen);

void
_curl_error_free(struct _curl_error *);

#endif /* _GLOBUS_AUTH_CURL_ERROR_H */
