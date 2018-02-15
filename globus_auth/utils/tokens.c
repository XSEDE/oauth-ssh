#include <string.h>
#include <stdio.h>

#include "globus_auth.h"

void
safe_print(const char * field, const char * value)
{
	printf("%s: %s\n", field, value ? value : "null");
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

		if (strcmp(grant_type, "introspect") == 0)
			return _introspect_token(argc, argv);
	}

	fprintf(stderr, "Usage: %s [introspect]\n", argv[0]);
	return 1;
}
