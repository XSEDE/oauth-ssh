#ifndef _ACCT_MAP_H_
#define _ACCT_MAP_H_

#include <globus_auth.h>

char **
acct_map_idp_suffix(const char * idp_suffix, const struct auth_identity **);

void
acct_map_free_list(char ** account_list);

#endif /* _ACCT_MAP_H_ */
