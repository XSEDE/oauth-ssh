#ifndef _STRINGS_H_
#define _STRINGS_H_

/*
 * System includes.
 */
#include <stdbool.h>
#include <stdarg.h>

#define CONST(type,var) (const type const *) var

// delimiter is ignored when set to (int)0
char *
join(const char * const strings[], const char delimiter);

// realloc's 'string' to fit suffix in
void
append(char ** string, const char * suffix);

void
insert(char *** array, const char * string);

char *
sformat(const char * format, ...);

bool
key_in_list(const char * const * list, const char * key);

bool
string_has_suffix(const char * string, const char * suffix);

void
free_array(char ** array);

#endif /* _STRINGS_H_ */
