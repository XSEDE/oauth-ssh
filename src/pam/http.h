#ifndef _HTTP_H_
#define _HTTP_H_

/*
 * Local includes.
 */
#include "credentials.h"

int
http_get_request(struct credentials *  credentials,
                 const char         *  request_url,
                 char               ** reply_body,
                 char               ** error_msg);

int
http_post_request(struct credentials *  credentials,
                  const char         *  request_url, 
                  const char         *  request_body,
                  char               ** reply_body,
                  char               ** error_msg);

#endif /* _HTTP_H_ */
