#ifndef _GLOBUS_AUTH_STRINGS_H
#define _GLOBUS_AUTH_STRINGS_H

#include <stdarg.h>
#include <string.h>

/*
 * Internal string helpers.
 */

#define _S_STR(s) (struct _string){s, 0} // 's' is static
#define _D_STR(s) (struct _string){s?strdup(s):NULL, s?1:0} // dup 's'
#define _F_STR(s) (struct _string){s, 1} // free 's'

struct _string {
	char * str;
	short  own_it; // !0 if we should free() it
};

// XXX circular dependency
#include "kv.h"

void
_string_free(struct _string s);

char *
_strings_build(const char * format, ...);

char *
_strings_build_list(char delimiter, const char ** value);

char *
_strings_build_kv_list(struct _kvs kvs, 
                       const char  delimiter, 
                       int         url_encode, 
                       int         sanitize);

char *
_strings_build_url(const  char * fqdn_n_path, 
                   struct _kvs   query_kvs,
                   int           sanitize);

char **
_strings_split_to_array(const char * string); // space delimited

#endif /* _GLOBUS_AUTH_STRINGS_H */
