#ifndef _CONFIG_H_
#define _CONFIG_H_

/*
 * System includes.
 */
#include <stdbool.h>

#define CONFIG_DEFAULT_FILE "/etc/oauth_ssh/globus-ssh.conf"

struct config {
	char *  client_id;
	char *  client_secret;
	char *  idp_suffix;
	char ** map_files;

	// Session support
	char ** permitted_idps;
	int     authentication_timeout;

	// 'Hidden' options set in PAM config file
	char *  environment; // default 'production'
	bool    debug;

        char ** auth_method;

#ifdef WITH_SCITOKENS        
	// Scitokens
	char ** issuers;
#endif
};

struct config * config_init(int flags, int argc, const char ** argv);
void config_fini(struct config *);

#endif /* _CONFIG_H_ */
