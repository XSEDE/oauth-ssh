#include <string.h>
#include <stdio.h>

#include "globus_auth.h"

void
safe_print(const char * field, const char * value)
{
	printf("%s: %s\n", field, value ? value : "null");
}

void
print_tokens(struct auth_token ** t)
{
	for (int i = 0; t && t[i]; i++)
	{
		if (t[i]->access_token) printf("access_token: %s\n", t[i]->access_token);
		if (t[i]->resource_server) printf("resource_server: %s\n", t[i]->resource_server);
		if (t[i]->token_type) printf("token_type: %s\n", t[i]->token_type);
		if (t[i]->refresh_token) printf("refresh_token: %s\n", t[i]->refresh_token);
		if (t[i]->id_token) printf("id_token: %s\n", t[i]->id_token);
		if (t[i]->state) printf("state: %s\n", t[i]->state);
		for (int j = 0; t[i]->scopes && t[i]->scopes[j]; j++)
		{
			printf("scope: %s\n", t[i]->scopes[j]);
		}

		printf("expires_in: %u\n", t[i]->expires_in);
	}
}

void
print_introspect(struct auth_introspect * i)
{
	printf("active: %d\n",     i->active);
	printf("expiry: %u\n",     i->expiry);
	printf("issued_at: %u\n",  i->issued_at);
	printf("not_before: %u\n", i->not_before);

	safe_print("display_name", i->display_name);
	safe_print("client_id",    i->client_id);
	safe_print("username",     i->username);
	safe_print("issuer",       i->issuer);
	safe_print("email",        i->email);
	safe_print("sub",          i->sub);

	printf ("identities: ");
	if (i->identities)
	{
		for (int j = 0; i->identities[j]; j++)
		{
			if (j) printf(",");
			printf(i->identities[j]);
		}
		printf("\n");
	} else
		printf("null\n");

	printf ("scopes: ");
	if (i->scopes)
	{
		for (int j = 0; i->scopes[j]; j++)
		{
			if (j) printf(",");
			printf(i->scopes[j]);
		}
		printf("\n");
	} else
		printf("null\n");

	printf ("audiences: ");
	if (i->audiences)
	{
		for (int j = 0; i->audiences[j]; j++)
		{
			if (j) printf(",");
			printf(i->audiences[j]);
		}
		printf("\n");
	} else
		printf("null\n");
}

#ifdef NOT
int
_code_grant(int argc, const char * argv[])
{
	if (argc != 7)
	{
		fprintf(stderr, "Usage: %s code <client_id> <client_secret> <code_verifier> <auth_code> <redirect_uri>\n", argv[0]);
		return 1;
	}

	const char * client_id     = argv[2];
	const char * client_secret = strlen(argv[3]) ? argv[3] : NULL;
	const char * code_verifier = strlen(argv[4]) ? argv[4] : NULL;
	const char * auth_code     = argv[5];
	const char * redirect_uri  = argv[6];

	struct auth_token ** tokens = NULL;

	auth_set_debug(1);

	int retval = auth_code_grant(client_id,
	                             client_secret,
	                             code_verifier,
	                             auth_code,
	                             redirect_uri,
	                             &tokens);

	print_tokens(tokens);
	auth_free_tokens(tokens);

	return retval;
}

int
_client_grant(int argc, const char * argv[])
{
	const char * client_id     = argv[2];
	const char * client_secret = argv[3];
	const char ** scopes = argv+4;

	if (argc < 5)
	{
		fprintf(stderr, "Usage: %s client <client_id> <client_secret> <scope1> ... <scopeN>\n", argv[0]);
		return 1;
	}

	return 1;
}

int
_dependent_grant(int argc, const char * argv[])
{
	const char * client_id     = argv[2];
	const char * client_secret = argv[3];
	const char * access_token  = argv[4];
	const char * access_type   = argv[5];

	if (argc != 6)
	{
		fprintf(stderr, "Usage: %s dependent <client_id> <client_secret> <access_token> <access_type>\n", argv[0]);
		return 1;
	}

	return 1;
}
#endif

int
_refresh_token(int argc, const char * argv[])
{
	const char * client_id     = argv[2];
	const char * client_secret = argv[3];
	const char * refresh_token = argv[4];

	if (argc != 5)
	{
		fprintf(stderr, "Usage: %s refresh <client_id> <client_secret> <refresh_token>\n", argv[0]);
		return 1;
	}

	if (strlen(client_secret) == 0)
		client_secret = NULL;

	auth_set_debug(1);

	struct auth_token ** tokens = NULL;
	struct auth_error *  auth_error = NULL;
	auth_error = auth_refresh_token(client_id, client_secret, refresh_token, &tokens);
	if (auth_error)
		printf("%s\n", auth_error->error_message);
	else
		print_tokens(tokens);

	auth_free_tokens(tokens);
	auth_free_error(auth_error);

	return 1;
}
#ifdef NOT

int
_revoke_token(int argc, const char * argv[])
{
	return 1;
}

int
_validate_token(int argc, const char * argv[])
{
	return 1;
}
#endif
int
_introspect_token(int argc, const char * argv[])
{
	const char * client_id          = argv[2];
	const char * client_secret      = argv[3];
	const char * access_token       = argv[4];
	const char * include_identities = argv[5];

	if (argc != 6)
	{
		fprintf(stderr, "Usage: %s %s <client_id> <client_secret> <access_token> <include_id_set>\n", argv[0], argv[1]);
		fprintf(stderr, "  where <include_id_set> is 'true' or 'false'\n");
		return 1;
	}

	//auth_set_debug(1);

	struct auth_introspect * introspect = NULL;
	struct auth_error * ae = auth_introspect_token(client_id,
	                                               client_secret,
	                                               access_token,
	                                               strcmp(include_identities, "true") ? 0 : 1,
	                                               &introspect);

	if (ae)
	{
		printf("%s\n", ae->error_message);
		auth_free_error(ae);
		return 1;
	}

	print_introspect(introspect);
	auth_free_introspect(introspect);

	return 0;
}

int
main(int argc, const char * argv[])
{
	if (argc > 1)
	{
		const char * grant_type = argv[1];

/*
		if (strcmp(grant_type, "code") == 0)
			return _code_grant(argc, argv);
		if (strcmp(grant_type, "client") == 0)
			return _client_grant(argc, argv);
		if (strcmp(grant_type, "dependent") == 0)
			return _dependent_grant(argc, argv);
*/
		if (strcmp(grant_type, "refresh") == 0)
			return _refresh_token(argc, argv);
/*
		if (strcmp(grant_type, "revoke") == 0)
			return _revoke_token(argc, argv);
		if (strcmp(grant_type, "validate") == 0)
			return _validate_token(argc, argv);
*/
		if (strcmp(grant_type, "introspect") == 0)
			return _introspect_token(argc, argv);
	}

	fprintf(stderr, "Usage: %s [code|client|refresh|revoke|validate|dependent|introspect]\n", argv[0]);
	return 1;
}
