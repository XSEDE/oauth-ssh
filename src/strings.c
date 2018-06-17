/*
 * System includes.
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*
 * Local includes.
 */
#include "strings.h"

char *
join(const char * const strings[], const char delimiter)
{
	if (!strings || !strings[0])
		return NULL;

	int length = 0;
	for (int i = 0; strings[i]; i++)
	{
		length += strlen(strings[i]) + 1;
	}

	char * joined_string = calloc(length, sizeof(char));

	int offset = 0;
	for (int i = 0; strings[i]; i++)
	{
		if (i != 0 && delimiter != 0)
			joined_string[offset++] = delimiter;
		strcpy(&joined_string[offset], strings[i]);
		offset += strlen(strings[i]);
	}

	return joined_string;
}

char *
concat(const char * string, const char * suffix)
{
	char * s = calloc(strlen(string) + strlen(suffix) + 1, sizeof(char));
	sprintf(s, "%s%s", string, suffix);
	return s;
}

char *
append(char ** string, const char * suffix)
{
	*string = realloc(*string, strlen(*string)+strlen(suffix)+1);
	strcat(*string, suffix);
	return *string;
}

char **
split(const char * string, char delimiter)
{
	char * string_copy = strdup(string);
	char * string_tmp  = string_copy;
	char * saveptr     = NULL;
	char   delim[2]    = {delimiter, 0};

	char ** strings    = NULL;
	int     count      = 0;
	char *  ptr        = NULL;
	while ((ptr = strtok_r(string_tmp, delim, &saveptr)))
	{
		string_tmp = NULL;
		strings = realloc(strings, (count+2)*sizeof(char *));
		strings[count++] = strdup(ptr);
		strings[count]   = NULL;
	}
	free(string_copy);

	return strings;
}

int
string_in_list(const char * string, const char * const list[])
{
	for (int i = 0; list && list[i]; i++)
	{
		if (strcmp(string, list[i]) == 0)
			return 1;
	}

	return 0;
}

char *
safe_strdup(const char * s)
{
	if (!s) return NULL;
	return strdup(s);
}
