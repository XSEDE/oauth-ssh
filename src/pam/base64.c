/*
 * System includes.
 */
#include <openssl/evp.h>
#include <stdlib.h>
#include <string.h>

/*
 * Local includes.
 */
#include "base64.h"
#include "debug.h" // always last

char *
base64_encode(const char * string)
{
	int length = ((strlen(string)/3) * 4) + 2;

	char * encoded_string = calloc(length, sizeof(char*));

	EVP_EncodeBlock((unsigned char *)encoded_string,
	                (const unsigned char *)string,
	                 strlen(string));
	return encoded_string;
}

char *
base64_decode(const char * string)
{
	char * decoded_string = calloc(strlen(string), sizeof(char*));

	int retval = EVP_DecodeBlock((unsigned char *)decoded_string,
	                             (const unsigned char *)string,
	                             strlen(string));

	if (retval != -1)
		return decoded_string;

	free(decoded_string);
	return NULL;
}
