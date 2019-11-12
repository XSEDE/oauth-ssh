/**
 * Public header for the SciTokens C library.
 *
 * 
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef void * SciTokenKey;
typedef void * SciToken;
typedef void * Validator;
typedef void * Enforcer;

typedef int (*StringValidatorFunction)(const char *value, char **err_msg);
typedef struct Acl_s {
    const char *authz;
    const char *resource;
}
Acl;

SciTokenKey scitoken_key_create(const char *key_id, const char *algorithm, const char *public_contents, const char *private_contents, char **err_msg);

void scitoken_key_destroy(SciTokenKey private_key);

SciToken scitoken_create(SciTokenKey private_key);

void scitoken_destroy(SciToken token);

int scitoken_set_claim_string(SciToken token, const char *key, const char *value, char **err_msg);

int scitoken_get_claim_string(const SciToken token, const char *key, char **value, char **err_msg);

int scitoken_get_expiration(const SciToken token, long long *value, char **err_msg);

void scitoken_set_lifetime(SciToken token, int lifetime);

int scitoken_serialize(const SciToken token, char **value, char **err_msg);

int scitoken_deserialize(const char *value, SciToken *token, char const* const* allowed_issuers, char **err_msg);

Validator validator_create();

int validator_add(Validator validator, const char *claim, StringValidatorFunction validator_func, char **err_msg);

int validator_add_critical_claims(Validator validator, const char **claims, char **err_msg);

int validator_validate(Validator validator, SciToken scitoken, char **err_msg);

Enforcer enforcer_create(const char *issuer, const char **audience, char **err_msg);

void enforcer_destroy(Enforcer);

int enforcer_generate_acls(const Enforcer enf, const SciToken scitokens, Acl **acls, char **err_msg);

void enforcer_acl_free(Acl *acls);

int enforcer_test(const Enforcer enf, const SciToken sci, const Acl *acl, char **err_msg);

#ifdef __cplusplus
}
#endif
