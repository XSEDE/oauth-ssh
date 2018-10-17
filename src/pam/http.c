/*
 * System includes.
 */
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

/*
 * Local includes.
 */
#include "http.h"
#include "strings.h"
#include "credentials.h"

enum {HTTP_GET, HTTP_POST};

struct response_body {
	int    length;
	char * body;
};

static size_t
capture_response(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	struct response_body * r = userdata;
	char * save_ptr = r->body;
	r->body = realloc(r->body, r->length + size*nmemb + 1);

	if (!r->body)
	{
		r->body = save_ptr;
		return 0;
	}

	if (r->length == 0)
		r->body[0] = '\0';

	strncat(r->body, ptr, size*nmemb);
	r->length += size*nmemb;

	return size * nmemb;
}

static int
http_request(struct credentials *  creds,
             int                   http_method,
             const char         *  request_url,
             const char         *  request_body,
             char               ** reply_body,
             char               ** error_msg)
{
	struct response_body response = {0, NULL};

	CURL * curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, capture_response);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA,     &response);
	curl_easy_setopt(curl, CURLOPT_URL,           request_url);

//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
	char * auth_header = NULL;
	struct curl_slist * list = NULL;

	switch (creds->type)
	{
	case BEARER:
		auth_header = concat("Authorization: Bearer ", creds->u.token);
		list = curl_slist_append(list, auth_header);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
		break;

	case CLIENT:
		curl_easy_setopt(curl, CURLOPT_USERNAME, creds->u.client.id);
		curl_easy_setopt(curl, CURLOPT_PASSWORD, creds->u.client.secret);
		break;
	}

	switch (http_method)
	{
	case HTTP_GET:
		curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
		break;

	case HTTP_POST:
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body);
		break;
	}

	/* Capture curl error descriptions. */
	char err_buf[CURL_ERROR_SIZE];
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, err_buf);

	CURLcode code = curl_easy_perform(curl);
	switch (code)
	{
	case 0:
		*reply_body = response.body;
		break;
	default:
		*error_msg = strdup(err_buf);
		break;
	}

	curl_easy_cleanup(curl);
	curl_slist_free_all(list);
	if (auth_header)
		free(auth_header);
	return !(code == 0);
}

/*
 * The reply could be:
 * {
 *  "message": "The resource could not be found.\n\n\n\n\n",
 *  "code"   : "404 Not Found",
 *  "title"  : "Not Found"
 *  }
 */
int
http_get_request(struct credentials *  credentials,
                 const char         *  request_url,
                 char               ** reply_body,
                 char               ** error_msg)
{
	return http_request(credentials,
	                    HTTP_GET,
	                    request_url,
	                    NULL,
	                    reply_body,
	                    error_msg);
}

int
http_post_request(struct credentials *  credentials,
                  const char         *  request_url, 
                  const char         *  request_body,
                  char               ** reply_body,
                  char               ** error_msg)
{
	return http_request(credentials,
	                    HTTP_POST,
	                    request_url,
	                    request_body,
	                    reply_body,
	                    error_msg);
}
