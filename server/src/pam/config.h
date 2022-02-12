#ifndef _CONFIG_H_
#define _CONFIG_H_

/*
 * System includes.
 */
#include <stdbool.h>

#define CONFIG_DEFAULT_FILE "/etc/oauth_ssh/oauth-ssh.conf"

typedef enum {
	GLOBUS_AUTH,
	SCITOKENS,
} auth_method_t;

struct config {

	//////
	// Determines which sections below are required
	//////
	char ** auth_method;

	// Hidden option in PAM config file. Increases logging output.
	bool    debug;

	//////
	// Globus Auth Section
	//////
	char *  environment; // Hidden option in PAM config file
	char *  client_id;
	char *  client_secret;
	char *  idp_suffix;
	char ** map_files;

	// Session support
	char ** permitted_idps;
	int     authentication_timeout;
	bool    mfa;

	//////
	// SciTokens Section
	//////
	char ** issuers;
};

struct config * config_init(int flags, int argc, const char ** argv);
void config_fini(struct config *);

// return true if the config is configured for the auth method
// false otherwise
bool config_auth_method(struct config *, auth_method_t);

#endif /* _CONFIG_H_ */
