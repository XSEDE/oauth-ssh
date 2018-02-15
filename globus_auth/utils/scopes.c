#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <globus_auth.h>

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
print_scope(struct auth_scope * scope)
{
	if (!scope)
	{
		printf("scope: null\n");
		return;
	}

	printf("==== Scope ====\n");

	safe_print("name",             scope->name);
	safe_print("description",             scope->description);
	safe_print("scope_suffix",             scope->scope_suffix);
	safe_print("id",             scope->id);
	safe_print("client",             scope->client);
	print_bool("allows_refresh_token", scope->allows_refresh_token);
	print_bool("advertised", scope->advertised);

	printf("=dependent_scopes=\n");
	for (int i = 0; scope->dependent_scopes && scope->dependent_scopes[i]; i++)
	{
		safe_print("scope",  scope->dependent_scopes[i]->scope);
		print_bool("optional", scope->dependent_scopes[i]->optional);
		print_bool("requires_refresh_token", scope->dependent_scopes[i]->requires_refresh_token);
	}
}

// XXX Generalize print routines to instead return strings that can be
// passed to printf(), syslog(), etc. Don't forget masking
void
print_scopes(struct auth_scope ** scopes)
{
        if (!scopes)
        {
                printf("scopes: null\n");
                return;
        }

        for (int i = 0; scopes[i]; i++)
        {
                printf("==== Scope %d ====\n", i);

				print_scope(scopes[i]);
        }
}


int
_bool(const char * value, int len)
{
	if (!len) len = strlen(value);

	if (strncmp(value, "true", len) == 0) return 1;
	if (strncmp(value, "false", len) == 0) return 0;

	fprintf(stderr, "Illegal boolean value: %.*s\n", len, value);
	exit (1);
}

int
_begins_with(const char * string, const char * prefix)
{
	return strncmp(string, prefix, strlen(prefix)) == 0;
}

void
_options_to_scope_struct(int                  argc, 
                         char              *  argv[], 
                         char              ** client_id, 
                         char              ** client_secret,
                         char              ** target_client,
                         struct auth_scope *  scope)
{
	memset(scope, 0, sizeof(*scope));

	int dep_scope_cnt = 0;

	for (int i = 0; i < argc; i++)
	{
		if (_begins_with(argv[i], "client_id="))
			*client_id = argv[i] + strlen("client_id=");
		else if (_begins_with(argv[i], "client_secret="))
			*client_secret = argv[i] + strlen("client_secret=");
		else if (_begins_with(argv[i], "target_client="))
			*target_client = argv[i] + strlen("target_client=");
		else if (_begins_with(argv[i], "name="))
			scope->name = argv[i] + strlen("name=");
		else if (_begins_with(argv[i], "description="))
			scope->description = argv[i] + strlen("description=");
		else if (_begins_with(argv[i], "scope_suffix="))
			scope->scope_suffix = argv[i] + strlen("scope_suffix=");

		else if (_begins_with(argv[i], "advertised="))
			scope->advertised = _bool(argv[i] + strlen("advertised="), 0);
		else if (_begins_with(argv[i], "allows_refresh_token="))
			scope->allows_refresh_token = _bool(argv[i] + strlen("allows_refresh_token="), 0);

		else if (_begins_with(argv[i], "dependent_scopes="))
		{
			scope->dependent_scopes = realloc(scope->dependent_scopes, 
			                                  sizeof(struct dependent_scope *) * (++dep_scope_cnt + 1));
			scope->dependent_scopes[dep_scope_cnt] = NULL;

			char * d0 = argv[i] + strlen("dependent_scopes=");
			char * d1 = strchr(d0+1, ':');
			char * d2 = strchr(d1+1, ':');

			scope->dependent_scopes[dep_scope_cnt-1] = calloc(sizeof(struct dependent_scope), 1);
			scope->dependent_scopes[dep_scope_cnt-1]->scope    = strndup(d0, d1-d0);
			scope->dependent_scopes[dep_scope_cnt-1]->optional = _bool(d1+1, d2-(d1+1));
			scope->dependent_scopes[dep_scope_cnt-1]->requires_refresh_token = _bool(d2+1, 0);
		} else
		{
			fprintf(stderr, "Illegal option: %s\n", argv[i]);
			exit (1);
		}
	}
}

void
_free_options(struct auth_scope * scope)
{
	for (int i = 0; scope->dependent_scopes[i]; i++)
	{
		free(scope->dependent_scopes[i]->scope);
		free(scope->dependent_scopes[i]);
	}
	if (scope->dependent_scopes) free(scope->dependent_scopes);
}

static int
_create_scopes(int argc, char * argv[])
{
	char * client_id     = NULL;
	char * client_secret = NULL;
	char * target_client = NULL;
	struct auth_scope    scope_def;
	struct auth_scope ** new_scopes;

	_options_to_scope_struct(argc-2, argv+2, &client_id, &client_secret, &target_client, &scope_def);

	if (!client_id || !client_secret || !target_client || !scope_def.name || !scope_def.description || !scope_def.scope_suffix)
	{
		fprintf(stderr, "Usage: %s create <options>\n", argv[0]);
		fprintf(stderr, "   ==== Valid options are: ====\n");
		fprintf(stderr, "   client_id=<uuid>        (REQUIRED)\n");
		fprintf(stderr, "   client_secret=<secret>  (REQUIRED)\n");
		fprintf(stderr, "   target_client=<uuid>    (REQUIRED)\n");
		fprintf(stderr, "   name=<string>           (REQUIRED)\n");
		fprintf(stderr, "   description=<string>    (REQUIRED)\n");
		fprintf(stderr, "   scope_suffix=<string>   (REQUIRED)\n");
		fprintf(stderr, "   advertised=[true|false] (Default: false)\n");
		fprintf(stderr, "   allows_refresh_token=[true|false] (Default: false)\n");
		fprintf(stderr, "   \n");
		fprintf(stderr, "You can specify zero or more of:\n");
		fprintf(stderr, "   dependent_scopes=<scope_id>:<optional>:<requires_refresh_token>\n");
		fprintf(stderr, "   where:\n");
		fprintf(stderr, "   <scope_id> is <string>\n");
		fprintf(stderr, "   <optional> is 'true' or 'false'\n");
		fprintf(stderr, "   <requires_refresh_token> is 'true' or 'false'\n");

		exit (1);
	}
	auth_set_debug(1);

	struct auth_error * ae = auth_create_scope(client_id, 
	                                    client_secret, 
	                                    target_client, 
	                                    scope_def, 
	                                    &new_scopes);
	print_scopes(new_scopes);

	return 1;
}

static int
_get_scopes(int argc, char * argv[])
{
	char *  owner_client_id     = argv[2];
	char *  owner_client_secret = argv[3];
	char ** scope_ids           = argv+4;

	if (argc < 4)
	{
		fprintf(stderr, "Usage: %s get <client id> <client secret> [<scope1> ... <scopeN>]\n", argv[0]);
		return 1;
	}

	auth_set_debug(1);

	struct auth_scope ** scopes = NULL;
	struct auth_error * ae = auth_get_scopes(owner_client_id, owner_client_secret, (const char **) scope_ids, &scopes);

	print_scopes(scopes);
	auth_free_scope(scopes);

	return 0;
}

static int
_update_scopes(int argc, char * argv[])
{
	fprintf(stderr, "Not implemented yet\n");
	return 1;
}

static int
_delete_scopes(int argc, char * argv[])
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
		if (strcmp(operation, "create") == 0)
			return _create_scopes(argc, argv);
		if (strcmp(operation, "get") == 0)
			return _get_scopes(argc, argv);

/*
		if (strcmp(operation, "update") == 0)
			return _update_scopes(argc, argv);

		if (strcmp(operation, "delete") == 0)
			return _delete_scopes(argc, argv);
*/
	}

	fprintf(stderr, "Usage: %s [create|get|update|delete]\n", argv[0]);
	return 1;
}
