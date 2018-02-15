#ifndef _GLOBUS_AUTH_ENCODE_H_
#define _GLOBUS_AUTH_ENCODE_H_

/*
 * Internal helpers for various encoding schemes.
 */

char *
_encode_base64(const unsigned char * InputValue, int InputLength);

char *
_encode_base64_url_safe(const unsigned char * InputValue, int InputLength);

char *
_encode_url_string(const char * InputValue, int InputLength);

char *
_encode_code_verifier(const char * CodeVerifier);

#endif /* _GLOBUS_AUTH_ENCODE_H_ */
