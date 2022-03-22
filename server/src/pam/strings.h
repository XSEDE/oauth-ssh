#ifndef _STRINGS_H_
#define _STRINGS_H_

/*
 * System includes.
 */
#include <stdbool.h>
#include <stdarg.h>

#define CONST(type,var) (const type const *) var


// Return a newly-allocated string whose contents consists of the
// entries of strings[] delimited by 'delimiter'. Returns NULL if
// ether strings[] or strings[0] are NULL. strings[] must be NULL-
// terminated. 'delimiter' is ignored when set to (int)0
char *
join(const char * const strings[], const char delimiter);

// realloc's 'string' to fit suffix in
void
append(char ** string, const char * suffix);

void
insert(char *** array, const char * string);

// Return a newly-allocated string whose contents are built with
// vsnprintf().
char *
sformat(const char * format, ...);

bool
key_in_list(const char * const * list, const char * key);

void
free_array(char ** array);

// Return a newly-allocated array whose contents are the 'delimiter'-delimited
// tokens of 'string'. Return NULL if 'string' is NULL. The elements of the
// array are also newly-allocated. The array will be NULL-terminated.
char **
split_string(const char * string, const char * delimiter);

#endif /* _STRINGS_H_ */
