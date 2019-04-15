/*
 * System includes.
 */
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>

/*
 * Local includes.
 */
#include "strings.h"
#include "logger.h"
#include "http.h"
#include "debug.h" // always last

typedef enum {HTTP_GET, HTTP_POST} request_type_t;

static size_t
capture_response(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	char * response = *(char **)userdata;

	*(char **)userdata = sformat("%s%.*s",
	                             response?response:"",
	                             size*nmemb,
	                             ptr);
	free(response);
	return size * nmemb;
}

static int
http_request(const char  *  client_id,
             const char  *  client_secret,
             request_type_t request_type,
             const char  *  request_url,
             const char  *  request_body,
             char        ** reply_body)
{
//	curl_global_init(CURL_GLOBAL_NOTHING); // prevent memory leaks
	CURL * curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, capture_response);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA,     reply_body);
	curl_easy_setopt(curl, CURLOPT_URL,           request_url);

//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
	curl_easy_setopt(curl, CURLOPT_USERNAME, client_id);
	curl_easy_setopt(curl, CURLOPT_PASSWORD, client_secret);

	switch (request_type)
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
		logger(LOG_TYPE_DEBUG, "%s", *reply_body ? *reply_body : "EMPTY");
		break;
	default:
		logger(LOG_TYPE_ERROR, "Globus Auth HTTP request failed: %s", err_buf);
		break;
	}
	curl_easy_cleanup(curl);
	return !(code == 0);
}

int
http_post_request(const struct config * config,
                  const char * request_url,
                  const char * request_body,
                  char ** reply_body)
{
	return http_request(config->client_id,
	                    config->client_secret,
	                    HTTP_POST,
	                    request_url,
	                    request_body,
	                    reply_body);
}

int
http_get_request(const struct config * config,
                 const char * request_url,
                 char ** reply_body)
{
	return http_request(config->client_id,
	                    config->client_secret,
	                    HTTP_GET,
	                    request_url,
	                    NULL,
	                    reply_body);
}
