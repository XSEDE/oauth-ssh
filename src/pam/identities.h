#ifndef _AUTH_IDENTITIES_H_
#define _AUTH_IDENTITIES_H_

/*
 * Local includes.
 */
#include "credentials.h"

/************************************************
 *
 * /v2/api/identities
 *
 ***********************************************/
typedef enum {ID_STATUS_UNUSED,
              ID_STATUS_USED,
              ID_STATUS_PRIVATE,
              ID_STATUS_CLOSED} identity_status_t;

struct identity {
	char            * id;
	char            * username;
	identity_status_t status;
	char            * identity_provider;
	char            * email;
	char            * name;
	char            * organization;
};

int
get_identities(struct credentials *   credentials,
               const char         **  ids_in,
               struct identity    *** ids_out,
               char               **  error_msg);

void
free_identities(struct identity **);

#endif /* _AUTH_IDENTITIES_H_ */
