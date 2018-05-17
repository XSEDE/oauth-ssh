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
 *   +<lineno> on parse error
 */
int
config_load(struct config * config,
            const char    * path);

int
config_save(const struct config * config, 
            const char          * path);

/*
 * Sections are unique and identified by section name. */

/*
 * ignored if section already exists.
 * otherwise, create an empty section.
 */
void
config_add_section(struct config * config, 
                   const char    * section);

/*
 * delete section and all key/values within the section. No complaints
 * if the section does not exist.
 */
void
config_del_section(struct config * config,
                   const char    * section);

/* caller must free the sections array
 * sections set to NULL if no sections exist.
 * all section names returned, list is null terminated, so are names
 * No guarantee on order of returned sections.
 */
void
config_get_sections(struct config *   config, 
                    char          *** sections);

int
config_section_exists(struct config * config, 
                      const char    * section);

// value can be null
// value updated if key exists
// section added if it does not exist
void
config_add_key(struct config * config,
               const char    * section, 
               const char    * key, 
               const char    * value);

// no complaint if section or key does not exist
void
config_del_key(struct config * config,
               const char    * section, 
               const char    * key);

// key order not guaranteed.
// caller must free keys
// keys is null terminated
void
config_get_keys(struct config *   config, 
                const char    *   section, 
                char          *** keys);

int
config_key_exists(struct config * config,
                  const char    * section,
                  const char    * key);

// value is null if section or key do not exist or value is null
void
config_get_value(struct config *  config,
                 const char    *  section,
                 const char    *  key,
                 char          ** value);

#endif /* _CONFIG_H_ */
