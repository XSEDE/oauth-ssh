#ifndef _BASE64_H_
#define _BASE64_H_

/*
 * System includes.
 */
#include <openssl/evp.h>

char *
base64_encode(const char * string);

char *
base64_decode(const char * string);

#endif /* _BASE64_H_ */

