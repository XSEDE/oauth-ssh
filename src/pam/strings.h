#ifndef _STRINGS_H_
#define _STRINGS_H_

// delimiter is ignored when set to (int)0
char *
join(const char * const strings[], const char delimiter);

char *
concat(const char * string, const char * suffix);

// realloc's 'string' to fit suffix in
char *
append(char ** string, const char * suffix);

char **
split(const char * string, const char delimiter);

int
string_in_list(const char * string, const char * const list[]);

char *
safe_strdup(const char * s);

#endif /* _STRINGS_H_ */
