#include <string.h>
#include <stdio.h>

#include "globus_auth.h"

void
safe_print(const char * label, const char * value)
{
	printf("%s: %s\n", label, value ? value : "null");
}

void
print_bool(const char * label, int value)
{
	printf("%s: %s\n", label, value ? "true" : "false");
}

void
print_array(const char * label, char ** array)
{
	if (!array)
	{
		printf("%s: null\n", label);
		return;
	}

	printf("==== %s ====\n", label);
	for (int i = 0; array[i]; i++)
	{
		printf("    %s\n", array[i]);
	}
}

void
print_clients(struct auth_client ** clients)
{
	if (!clients)
	{
		printf("clients: null\n");
		return;
	}

	for (int i = 0; clients[i]; i++)
	{
		printf("==== Client %d ====\n", i);

		safe_print("name",             clients[i]->name);
		safe_print("terms_of_service", clients[i]->terms_of_service);
		safe_print("privacy_policy",   clients[i]->privacy_policy);
		safe_print("required_idp",     clients[i]->required_idp);
		safe_print("preselect_idp",    clients[i]->preselect_idp);
		safe_print("id",               clients[i]->id);
		safe_print("parent_id",        clients[i]->parent_id);
		safe_print("project_id",       clients[i]->project_id);
		print_array("redirect_uris",   clients[i]->redirect_uris);
		print_array("grant_types",     clients[i]->grant_types);
		print_array("scopes",          clients[i]->scopes);
		print_array("fqdns",           clients[i]->fqdns);

		switch (clients[i]->visibility)
		{
		case CLIENT_VIS_PUBLIC:
			safe_print("visibility", "public");
			break;
		case CLIENT_VIS_PRIVATE:
			safe_print("visibility", "private");
			break;
		}
	}
}

static int
_create_clients(int argc, char * argv[])
{
	fprintf(stderr, "Not implemented yet\n");
	return 1;
}

static int
_get_clients(int argc, char * argv[])
{
	const char * client_id      = argv[2];
	const char * client_secret  = argv[3];

	if (argc != 4 && argc != 5)
	{
		fprintf(stderr, "Usage: %s get <client id> <client secret> [<client id or fqdn>]\n", argv[0]);
		return 1;
	}

	auth_set_debug(1);

	struct auth_client ** auth_clients = NULL;
	struct auth_error * ae = auth_get_client(client_id, 
	                                         client_secret, 
	                                         argc == 5 ? argv[4] : NULL, 
	                                         &auth_clients);

	print_clients(auth_clients);
	auth_free_clients(auth_clients);

	return 0;
}

static int
_update_clients(int argc, char * argv[])
{
	fprintf(stderr, "Not implemented yet\n");
	return 1;
}

static int
_add_fqdn(int argc, char * argv[])
{
	fprintf(stderr, "Not implemented yet\n");
	return 1;
}

static int
_delete_clients(int argc, char * argv[])
{
	fprintf(stderr, "Not implemented yet\n");
	return 1;
}

int
main(int argc, char * argv[])
{
	const char * operation = argv[1];

	if (argc > 1)
	{
/*
		if (strcmp(operation, "create") == 0)
			return _create_clients(argc, argv);
*/
		if (strcmp(operation, "get") == 0)
			return _get_clients(argc, argv);
/*
		if (strcmp(operation, "update") == 0)
			return _update_clients(argc, argv);
		if (strcmp(operation, "add_fqdn") == 0)
			return _add_fqdn(argc, argv);
		if (strcmp(operation, "delete") == 0)
			return _delete_clients(argc, argv);
*/
	}

	fprintf(stderr, "Usage: %s [create|get|update|add_fqdn|delete]\n", argv[0]);
	return 1;
}
