#ifndef _CONFIG_H_
#define _CONFIG_H_

struct config;

struct config * config_init();

void
config_free(struct config * config);

/*
 * Returns:
 *   -<errno> on system error
 *    0 on success
 */
int
config_load(struct config * config,
            const char    * path);

// value is null if or key does not exist or value is null
void
config_get_value(struct config *  config,
                 const char    *  key,
                 char          ** value);

#endif /* _CONFIG_H_ */
