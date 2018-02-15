#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "globus_auth.h"

const int CodeVerifierLength=128;

int
main(int argc, const char * argv[])
{
	const char *  client_id     = argv[1];
	const char *  client_secret = argv[2];
	const char *  redirect_url  = argv[3];
	const char ** scopes = argv+4;

	if (argc < 5)
	{
		fprintf(stderr, "Usage: %s <client_id> <client_secret> <redirect_url> <scope1> ... <scopeN>\n", argv[0]);
		return 1;
	}

	if (strlen(client_secret) == 0)
		client_secret = NULL;

	char * code_verifier = NULL;
	if (!client_secret)
		code_verifier = auth_code_verifier(CodeVerifierLength);

	char * consent_url = auth_code_grant_url(client_id,
	                                         code_verifier,
	                                         redirect_url,
	                                         scopes,
	                                        "_default",
	                                         AUTH_ACCESS_TYPE_OFFLINE);

	printf("Please visit %s\n", consent_url);
	printf("Code verifier: %s\n", code_verifier);
	free(consent_url);
	free(code_verifier);
	return 0;
}
