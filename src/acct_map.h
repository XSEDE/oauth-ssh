#ifndef _ACCT_MAP_H_
#define _ACCT_MAP_H_

struct acct_map;

struct acct_map * acct_map_init();

void acct_map_free(struct acct_map*);

int acct_map_add_module(struct acct_map *,
                        const char * module, 
                        const char * option);

char **
acct_map_lookup(struct acct_map * acct_map, char * usernames_or_ids[]);

#endif /* _ACCT_MAP_H_ */
