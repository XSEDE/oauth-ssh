/*
 * Copyright The University of Chicago
 * All Rights Reserved.
 */
#ifndef ACCOUNT_MAPPING_H
#define ACCOUNT_MAPPING_H

char **
acct_map_id_to_accts(const char *const* auth_identities);

void
acct_map_free_list(char ** account_list);

#endif /* ACCOUNT_MAPPING_H */
