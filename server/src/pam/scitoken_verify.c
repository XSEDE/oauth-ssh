#include <stdlib.h>
#include <string.h>

#include <scitokens-cpp/src/scitokens.h>
int scitoken_verify(const char * auth_line){

    SciToken scitoken;
    char *err_msg;
    const char* listofauthz= "COPY:write DELETE:write GET:read HEAD:read LOCK:write MKCOL:write MOVE:write OPTIONS:read POST:read PROPFIND:write PROPPATCH:write PUT:write TRACE:read UNLOCK:write";

    if(auth_line == NULL)
    {
        logger(LOG_TYPE_INFO,
           "Token == NULL");
        return 0;
    }
    
    char *auth_scheme = strtok((char *)auth_line, " ");
    //This is the actual token
    auth_line = strtok(NULL, " ");
    while (*auth_line==' ') auth_line++;
    int numberofissuer = 1;//TODO: Harcoded varible
    char *null_ended_list[numberofissuer+1];

    if (strcasecmp(auth_scheme, "Bearer"))
    {
        logger(LOG_TYPE_INFO,
            "Wrong scheme");
        return 0;
    }
    
    //Read list of issuers from configuration and create a null ended list of strings
    char *issuer = "https://demo.scitokens.org";//TODO: Harcoded varible
    for(int i = 0; i<numberofissuer; i++) null_ended_list[i] = issuer;
    null_ended_list[numberofissuer] = NULL;

    if(sizeof(auth_line)>1000*1000)
    {
        logger(LOG_TYPE_INFO,
        "SciToken too large");
        return 0;
    }

    if(scitoken_deserialize(auth_line, &scitoken, (const char * const*)null_ended_list, &err_msg))
    {
        logger(LOG_TYPE_INFO,
        "Failed to deserialize scitoken %s \n %s",
        auth_line,err_msg);
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
    acl.authz = "";
    acl.resource = "";
    
    //Retrieve request type => acl.authz = read/write
    char* requesttype = "GET";//TODO: Harcoded varible, in apache module, r->method;
    char* authzsubstr = strstr(listofauthz,requesttype);
    
    if(authzsubstr == NULL){
        logger(LOG_TYPE_INFO,
        "Request type not supported(acl.authz)");
        return 0;
    }
    
    char *substr = (char *)calloc(1, strchr(authzsubstr,' ') - authzsubstr + 1);
    memcpy(substr,authzsubstr,strchr(authzsubstr,' ') - authzsubstr);
    strtok(substr,":");
    acl.authz = strtok(NULL,":");
    
    //Resource is found/not found for the audience
    int found = 0;
    
    for(int i=0; i<2; i++)//TODO: Harcoded varible 2, numberofissuer
    {
        //This for loop is suppose to search for the resource associated with the audience(in the config file)
        acl.resource = "/path";//TODO: Harcoded varible
        found = 1;
    }
    
    if(!found)
    {
        logger(LOG_TYPE_INFO,
        "Resource not found");
        free(substr);
        return 0;
    }

    if (enforcer_test(enf, scitoken, &acl, &err_msg))
    {
        logger(LOG_TYPE_INFO,
        "Failed enforcer test %s %s %s %s",
        err_msg, acl.authz, acl.resource,aud_list[0]);
        free(err_msg);
        free(substr);
        return 0;
    }

    free(substr);
    return 1;
}
