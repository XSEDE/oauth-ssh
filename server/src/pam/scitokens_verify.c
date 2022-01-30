/*
 * System includes.
 */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <scitokens/scitokens.h>
/*
 * Local includes.
 */
#include "logger.h"
#include "config.h"
int scitoken_verify(const char * auth_line, const struct config * config, const char * scitoken_requested_user)
{
    SciToken scitoken;
    char *err_msg;

    if(auth_line == NULL)
    {
        logger(LOG_TYPE_INFO,
        	"Token == NULL");
        return 0;
    }

    if(sizeof(config->issuers) == 0)
    {
        logger(LOG_TYPE_INFO,
		"No issuers in config");
	return 0;
    }

    size_t numberofissuers = sizeof(config->issuers)/sizeof(config->issuers[0]);

    char *null_ended_list[numberofissuers+1];

    //Read list of issuers from configuration and create a null ended list of strings
    for(size_t i = 0; i<numberofissuers; i++) null_ended_list[i] = config->issuers[i];
    null_ended_list[numberofissuers] = NULL;

    if(sizeof(auth_line)>1000*1000)
    {
        logger(LOG_TYPE_INFO,
        	"SciToken too large");
        return 0;
    }

    if(scitoken_deserialize(auth_line, &scitoken, (const char * const*)null_ended_list, &err_msg))
    {
        logger(LOG_TYPE_INFO,
        	"Failed to deserialize scitoken %s \n %s + %s",
		auth_line,err_msg,config->issuers[0]);
        free(err_msg);
        return 0;
    }

    char* issuer_ptr = NULL;
    if(scitoken_get_claim_string(scitoken, "iss", &issuer_ptr, &err_msg))
    {
        logger(LOG_TYPE_INFO,
        	"Failed to get claim \n %s",
        	err_msg);
        free(err_msg);
        return 0;
    }

    //Preparing for enforcer test
    Enforcer enf;
    char hostname[1024];
    const char* aud_list[2];

    //Retrieve the hostname for the audience. It is using hostname(NOT domain name). Set payload accordingly
    if (gethostname(hostname, 1024) != 0)
    {
        logger(LOG_TYPE_INFO,
        	"Failed to get hostname");
        return 0;
    }
    aud_list[0] = hostname;
    aud_list[1] = NULL;

    if (!(enf = enforcer_create(issuer_ptr, aud_list, &err_msg)))
    {
        logger(LOG_TYPE_INFO,
        	"Failed to create enforcer\n %s",
        	err_msg);
        free(err_msg);
        return 0;
    }

    Acl acl;
    acl.authz = "ssh";
    acl.resource = scitoken_requested_user;

    if (enforcer_test(enf, scitoken, &acl, &err_msg))
    {
	logger(LOG_TYPE_INFO,
		"Failed enforcer test %s %s %s %s",
		err_msg, acl.authz, acl.resource,aud_list[0]);
	free(err_msg);
        return 0;
    }

    return 1;
}
