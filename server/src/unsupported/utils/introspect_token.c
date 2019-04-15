/*
 * System includes.
 */
#include <string.h>
#include <unistd.h>
#include <stdio.h>

/*
 * Local includes.
 */
#include "introspect.h"
#include "credentials.h"
#include "config.h"

static int
load_config(const char * config_file, char ** client_id, char ** client_secret);

void
usage(const char * program)
{
	fprintf(stderr, "Usage: %s [options] <token>\n", program);
	fprintf(stderr, "\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "\t-i <client_id>:\n");
	fprintf(stderr, "\t-s <client_secret>:\n");
	fprintf(stderr, "\t-c <config_file>:\n");
}

const char *
safe_string(const char * s)
{
	if (!s) return "null";
	return s;
}

/*
 * Usage: identities [options] <id1> [<id2>...<idN>]
 *
 * Options:
 *    -i <client_id>
 *    -s <client_secret>
 *    -c <config_file>
 */

int
main(int argc, char * argv[])
{
	char * client_id     = NULL;
	char * client_secret = NULL;
	char * config_file   = NULL;
	char * token         = NULL;

	int opt;
	while ((opt = getopt(argc, argv, "i:s:c:")) != -1)
	{
		switch (opt)
		{
		case 'i':
			client_id = optarg;
			break;
		case 's':
			client_secret = optarg;
			break;
		case 'c':
			config_file = optarg;
			break;
		default:
			usage(argv[0]);
			return 1;
		}
	}

	token = argv[optind];

	if (!config_file)
		config_file = "/etc/globus/globus-ssh.conf";

	if (!client_id || !client_secret)
	{
// XXX what about free'ing these returned values
		int retval = load_config(config_file,
		                         client_id     ? NULL : &client_id, 
		                         client_secret ? NULL : &client_secret);
		if (retval)
			return 1;
	}

	if (!client_id || !client_secret)
	{
		fprintf(stderr,
		        "Please specify the %s%s%s\n",
		        client_id     ? "" : "client_id",
		        !client_id && !client_secret ? "and" : "",
		        client_secret ? "" : "client_secret");
		usage(argv[0]);
		return 1;
	}

	struct credentials creds = init_client_creds(client_id, client_secret);
	struct introspect * resource = NULL;

	char * errmsg = NULL;
	int retval = introspect(&creds, token, &resource, &errmsg);
	if (retval)
	{
		fprintf(stderr, "%s\n", errmsg);
		return 1;
	}

	printf("active : %d\n", resource->active);
	printf("scopes :\n");
	for (int i = 0; resource->scopes && resource->scopes[i]; i++)
	{
		printf("\t%s\n", resource->scopes[i]);
	}
	printf("sub    : %s\n", safe_string(resource->sub));
	printf("username : %s\n", safe_string(resource->username));
	printf("display_name : %s\n", safe_string(resource->username));
	printf("email : %s\n", safe_string(resource->email));
	printf("client_id : %s\n", safe_string(resource->email));

	printf("audiences :\n");
	for (int i = 0; resource->audiences && resource->audiences[i]; i++)
	{
		printf("\t%s\n", resource->audiences[i]);
	}
	printf("issuer : %s\n", safe_string(resource->issuer));

	printf("expiry : %lu\n", resource->expiry);
	printf("issued_at : %lu\n", resource->issued_at);
	printf("not_before : %lu\n", resource->not_before);

	printf("identities :\n");
	for (int i = 0; resource->identities && resource->identities[i]; i++)
	{
		printf("\t%s\n", resource->identities[i]);
	}

	free_introspect(resource);

	return 0;
}

static int
load_config(const char * config_file, char ** client_id, char ** client_secret)
{
	struct config * config = config_init();

	int retval = config_load(config, config_file);
	if (retval)
	{
		fprintf(stderr, "Failed to open %s: %s\n", config_file, strerror(-retval));
		return 1;
	}

	if (client_id)
		config_get_value(config, "client_id", client_id);

	if (client_secret)
		config_get_value(config, "client_secret", client_secret);

	config_free(config);

	return 0;
}

