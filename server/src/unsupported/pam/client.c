#define PAM_SM_AUTH
#include <security/pam_modules.h>
#include <assert.h>
#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

const char * PAM_MODULE="../../pam/.libs/pam_globus.so";

const char * Token = NULL;

static int
_pam_conv_func(int num_msg,
               const struct pam_message **msg,
               struct pam_response **resp,
               void *appdata_ptr)
{
	switch (msg[0]->msg_style)
	{
	case PAM_PROMPT_ECHO_OFF:
		*resp = calloc(1, sizeof(**resp));
		(*resp)->resp = Token ? strdup(Token) : NULL;
		break;
	case PAM_TEXT_INFO:
		printf("%s\n", msg[0]->msg);
		break;
	}

	return PAM_SUCCESS;
}

static struct pam_conv _pam_conv = {_pam_conv_func, NULL};

int
pam_get_item(const pam_handle_t *pamh, int item_type, const void **item)
{
	//printf("%s\n", __func__);
	switch (item_type)
	{
	case PAM_CONV:
		*item = &_pam_conv;
		break;

	default:
		assert(0);
	}

	return PAM_SUCCESS;
}

const char * RequestedUser = NULL;

int
pam_get_user(pam_handle_t * pamh, const char **user, const char *prompt)
{
	//printf("%s\n", __func__);
	*user = RequestedUser;
	return PAM_SUCCESS;
}

int
main()
{
	void * module = dlopen(PAM_MODULE, RTLD_NOW);
	if (!module)
	{
		fprintf(stderr, "Failed to load the PAM module: %s\n", dlerror());
		return 1;
	}

	int (*pam_auth)(pam_handle_t * pam, int flags, int argc, const char **argv);

	pam_auth = dlsym(module, "pam_sm_authenticate");
	if (!module)
	{
		fprintf(stderr,"Failed to load pam_sm_authenticate(): %s\n",dlerror());
		return 1;
	}

	const char * _argv[] = {"environment=sandbox", "debug", NULL};
	RequestedUser = "centos";
	Token = "AgbnNEQ3Yr6MqWXbkXEe84GmNM09j5a8rYxdaVd1VPoq9qbeJnC5Cq23Wvab7x78WybQ1b177dno49sDBqG2QUXn5";

	assert(pam_auth(NULL, 0, 2, _argv) == PAM_SUCCESS);

	RequestedUser = "bob";
	Token = "AgbnNEQ3Yr6MqWXbkXEe84GmNM09j5a8rYxdaVd1VPoq9qbeJnC5Cq23Wvab7x78WybQ1b177dno49sDBqG2QUXn5";
	assert(pam_auth(NULL, 0, 2, _argv) == PAM_AUTH_ERR);

	RequestedUser = "centos";
	Token = "XXXXXX";
	assert(pam_auth(NULL, 0, 2, _argv) == PAM_AUTH_ERR);

	RequestedUser = "root";
	Token = "AgbnNEQ3Yr6MqWXbkXEe84GmNM09j5a8rYxdaVd1VPoq9qbeJnC5Cq23Wvab7x78WybQ1b177dno49sDBqG2QUXn5";
	assert(pam_auth(NULL, 0, 2, _argv) == PAM_AUTH_ERR);

	RequestedUser = "root";
	// echo '{"command": {"op": "get_security_policy"}}' | base64
	Token = "eyJjb21tYW5kIjogeyJvcCI6ICJnZXRfc2VjdXJpdHlfcG9saWN5In19Cg==";
	assert(pam_auth(NULL, 0, 2, _argv) == PAM_MAXTRIES);

	RequestedUser = "centos";
	// echo '{"command": {"op": "blahblahblah"}}' | base64
	Token = "eyJjb21tYW5kIjogeyJvcCI6ICJibGFoYmxhaGJsYWgifX0K";
	assert(pam_auth(NULL, 0, 2, _argv) == PAM_AUTHINFO_UNAVAIL);

	return 0;
}
