#include <string.h>
#include <stdio.h>
#include <globus_auth.h>

int
main(int argc, const char * argv[])
{
	const char *  client_id        = argv[1];
	const char *  client_secret    = argv[2];
	const char ** ids_or_usernames = argv + 3;

	if (argc < 4)
	{
		fprintf(stderr, "Usage: %s <client_id> <client_secret> <id1> ... <idN>\n", argv[0]);
		return 1;
	}

	if (strlen(client_id) == 0)
		client_id = NULL;

	auth_set_debug(1);

	struct auth_identity ** ids = NULL;
	struct auth_error * ae = auth_get_identities(client_id, client_secret, ids_or_usernames, &ids);

	for (int i = 0; ids && ids[i]; i++)
	{
		printf("id: %s\n",       ids[i]->id           ? ids[i]->id : "null");
		printf("username: %s\n", ids[i]->username     ? ids[i]->username : "null");

		switch (ids[i]->status)
		{
		case ID_STATUS_UNUSED:
			printf("status: unused\n");
			break;
		case ID_STATUS_USED:
			printf("status: used\n");
			break;
		case ID_STATUS_PRIVATE:
			printf("status: private\n");
			break;
		case ID_STATUS_CLOSED:
			printf("status: closed\n");
			break;
		}

		printf("email: %s\n",    ids[i]->email        ? ids[i]->email : "null");
		printf("name: %s\n",     ids[i]->name         ? ids[i]->name : "null");
		printf("org: %s\n",      ids[i]->organization ? ids[i]->organization : "null");
	}

	auth_free_identities(ids);

	return 0;
}

