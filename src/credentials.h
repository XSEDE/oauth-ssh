#ifndef _CREDENTIALS_H_
#define _CREDENTIALS_H_

struct credentials {
	enum {BEARER, CLIENT} type;
	union {
		const char * token;
		struct {
			const char * id;
			const char * secret;
		} client;
	} u;
};


struct credentials
init_bearer_creds(const char * bearer_token);

struct credentials
init_client_creds(const char * client_id, const char * client_secret);

#endif /* _CREDENTIALS_H_ */
