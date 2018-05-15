#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "curl_wrapper.h"
#include "request.h"
#include "response.h"

#include "test/unit_test.h"

/**********************************************************
 *
 * Debugging routines.
 *
 *********************************************************/
static int _curl_debug = 0;

void
_curl_set_verbose(int debug) // print to stderr if !debug
{
	_curl_debug = (debug != 0);
}

/**********************************************************
 *
 * Helpers for receiving the response body (if one exists)
 *
 *********************************************************/
static size_t
_read_response(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	struct _response * r = userdata;
	char * save_ptr      = r->body;
	r->body = realloc(r->body, r->length + size*nmemb + 1);

	if (!r->body)
	{
		r->body = save_ptr;
		return 0;
	}

	if (r->length == 0)
		r->body[0] = '\0';

	strcat(r->body, ptr);
	r->length += size*nmemb;

	return size * nmemb;
}

/**********************************************************
 *
 * Helpers for sending the request body (if one exists)
 *
 *********************************************************/
struct _curl_request {
	int  offset;
	char * body;
};

static size_t 
_write_request(void * ptr, size_t size, size_t nmemb, void * userdata)
{
	struct _curl_request * creq = userdata;
	int length = strlen(creq->body) - creq->offset;
	if (length > size * nmemb)
		length = size * nmemb;
	memcpy(ptr, creq->body + creq->offset, length);
	creq->offset += length;
	return length;
}

/**********************************************************
 *
 * Execute the request against the API endpoint.
 *
 *********************************************************/

struct _curl_error *
_curl_send_request(const char          * client_id, // NULL if bearer token used
                   const char          * secret_or_bearer_token,
                   const struct _request request,
                   struct _response    * response)
{
	char * body = _request_build_body(request, 0);
	char * url  = _request_build_url(request, 0);

	struct _curl_request creq = {body ? strlen(body) : 0, body};

	memset(response, 0, sizeof(*response));

	CURL * curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _read_response);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA,     response);
	curl_easy_setopt(curl, CURLOPT_URL,           url);

	if (_curl_debug)
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

	if (client_id)
	{
		curl_easy_setopt(curl, CURLOPT_USERNAME, client_id);
		if (secret_or_bearer_token)
			curl_easy_setopt(curl, CURLOPT_PASSWORD, secret_or_bearer_token);
	} 

	/* Capture curl error descriptions. */
	char err_buf[CURL_ERROR_SIZE];
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER,   err_buf);

	char * auth_header = NULL;
	struct curl_slist *list = NULL;
	if (!client_id)
	{
		ASSERT(secret_or_bearer_token);
		// libcurl-devel-7.29.0-42.el7_4.1.x86_64 does not support CURLOPT_XOAUTH2_BEARER
		auth_header = _strings_build("Authorization: Bearer %s", secret_or_bearer_token);
		list = curl_slist_append(list, auth_header);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
	}

	switch (request.op)
	{
	case REQUEST_OP_POST:
		ASSERT(body);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
		break;
	case REQUEST_OP_GET:
		curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
		break;
	case REQUEST_OP_PUT:
		ASSERT(creq.body);
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, _write_request);
		curl_easy_setopt(curl, CURLOPT_READDATA, &creq);
		curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)strlen(creq.body));
		break;
	case REQUEST_OP_DELETE:
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
		break;
	}

 	CURLcode code = curl_easy_perform(curl);
	struct _curl_error * ce = _curl_error(code, err_buf, sizeof(err_buf));
	curl_easy_cleanup(curl);

	curl_slist_free_all(list);
	if (auth_header)
		free(auth_header);

	if (_curl_debug)
	{
// XXX This should optional print elsewhere or even just return the information for logging
		if (body) printf("%s\n", body);
		if (response->body) printf("%s\n", response->body);
	}

	if (body) free(body);
	if (url)  free(url);

	return ce;
}

/*
 * curl_easy_escape() but safe for HTML forms.
 */
char *
_curl_safe_escape(const char * InputValue, int InputLength)
{
	CURL * curl = curl_easy_init();
	char * curl_string = curl_easy_escape(curl, InputValue, InputLength);

        char * string = strdup(curl_string);
        curl_free(curl_string);
	curl_easy_cleanup(curl);
        return string;
}
