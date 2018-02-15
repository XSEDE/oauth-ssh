#include <openssl/sha.h>
#include <openssl/evp.h>
#include <curl/curl.h>
#include <string.h>

#include "encode.h"
#include "tests/unit_test.h"

char *
_encode_base64(const unsigned char * InputValue, int InputLength)
{
	int output_value_length = (InputLength / 3) * 4;
	if ((InputLength % 3) != 0)
		output_value_length += 4;

	char * encoded_value = calloc(output_value_length+1, sizeof(char));

	EVP_EncodeBlock((unsigned char *)encoded_value, InputValue, InputLength);
	return encoded_value;
}

char *
_encode_base64_url_safe(const unsigned char * InputValue, int InputLength)
{
	char * base64_encoded = _encode_base64(InputValue, InputLength);

	for (int i = 0; i < strlen(base64_encoded); i++)
	{
		switch(base64_encoded[i])
		{
		case '+':
			base64_encoded[i] = '-';
			break;
		case '/':
			base64_encoded[i] = '_';
			break;
		default:
			break;
		}
	}
	return base64_encoded;
}

char *
_encode_url_string(const char * InputValue, int InputLength)
{
	CURL * curl = curl_easy_init();
	char * curl_string = curl_easy_escape(curl, InputValue, InputLength);
	char * string = strdup(curl_string);
	curl_free(curl_string);
	return string;
}

char *
_encode_code_verifier(const char * CodeVerifier)
{
	unsigned char sha256_digest[SHA256_DIGEST_LENGTH];

	SHA256((const unsigned char *)CodeVerifier, strlen(CodeVerifier), sha256_digest);
	char * base64_encoded = _encode_base64_url_safe(sha256_digest, sizeof(sha256_digest));

	while (base64_encoded[strlen(base64_encoded)-1] == '=')
	{
		base64_encoded[strlen(base64_encoded)-1] = '\0';
	}

	return base64_encoded;
}
