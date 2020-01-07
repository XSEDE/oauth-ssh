#ifndef _SVERIFY_H_
#define _SVERIFY_H_

/*
 * System includes.
 */
#include <stdlib.h>
#include <string.h>
#include <scitokens/scitokens.h>
int
scitoken_verify(const char * auth_line, const struct config * config, const char * scitoken_requested_user);

#endif /* _SVERIFY_H_ */
