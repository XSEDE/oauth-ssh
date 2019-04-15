#ifndef _HTTP_H_
#define _HTTP_H_

/*
 * Local includes.
 */
#include "config.h"

int
http_post_request(const struct config * config,
                  const char * request_url,
                  const char * request_body,
                  char ** reply_body);

int
http_get_request(const struct config * config,
                 const char * request_url,
                 char ** reply_body);

#endif /* _HTTP_H_ */
