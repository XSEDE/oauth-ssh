/*
 * System includes.
 */
#include <string.h>
#include <unistd.h>
#include <stdio.h>

/*
 * Local includes.
 */
#include "identities.h"
#include "credentials.h"
#include "config.h"

static int
load_config(const char * config_file, char ** client_id, char ** client_secret);

void
usage(const char * program)
{
	fprintf(stderr, "Usage: %s [options] <id1> [<id2>...<idN>]\n", program);
	fprintf(stderr, "\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "\t-i <client_id>:\n");
	fprintf(stderr, "\t-s <client_secret>:\n");
	fprintf(stderr, "\t-c <config_file>:\n");
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
	char *  client_id     = NULL;
	char *  client_secret = NULL;
	char *  config_file   = NULL;
	const char ** ids     = NULL;

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

	ids = (const char **) &(argv[optind]);

	if (!config_file)
		config_file = "/etc/globus/globus-ssh.conf";

	if (!client_id || !client_secret)
	{
// XXX what about free'ing these returned values
		int retval = load_config(config_file,
		                         client_id ? NULL : &client_id, 
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
	struct identity ** auth_ids = NULL;

	char * errmsg = NULL;
	int retval = get_identities(&creds, ids, &auth_ids, &errmsg);

	if (retval)
	{
		fprintf(stderr, "%s\n", errmsg);
		return 1;
	}

	for (int i = 0; auth_ids && auth_ids[i]; i++)
	{
		printf("id: %s\n",       auth_ids[i]->id       ? auth_ids[i]->id : "null");
		printf("username: %s\n", auth_ids[i]->username ? auth_ids[i]->username : "null");

		switch (auth_ids[i]->status)
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

		printf("email: %s\n",    auth_ids[i]->email        ? auth_ids[i]->email : "null");
		printf("name: %s\n",     auth_ids[i]->name         ? auth_ids[i]->name : "null");
		printf("org: %s\n",      auth_ids[i]->organization ? auth_ids[i]->organization : "null");
		printf("idp: %s\n",      auth_ids[i]->identity_provider ? auth_ids[i]->identity_provider : "null");
	}

	free_identities(auth_ids);

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

