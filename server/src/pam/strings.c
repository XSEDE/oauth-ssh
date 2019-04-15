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
#include "debug.h" // always last

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

void
append(char ** string, const char * suffix)
{
	if (*string == NULL)
	{
		*string = strdup(suffix);
		return;
	}

	*string = realloc(*string, strlen(*string)+strlen(suffix)+1);
	strcat(*string, suffix);
}

void
insert(char *** array, const char * string)
{
	int cnt = 0;
	for (cnt = 0; *array && (*array)[cnt]; cnt++)
	{
		if (strcmp((*array)[cnt], string) == 0)
			return;
	}

	*array = realloc(*array, (cnt+2)*sizeof(**array));
	(*array)[cnt] = strdup(string);
	(*array)[cnt+1] = NULL;
}

char *
sformat(const char * format, ...)
{
	va_list ap;

	va_start(ap, format);
	int len = vsnprintf(NULL, 0, format, ap);
	va_end(ap);

	char * str = calloc(len+1, sizeof(char));

	va_start(ap, format);
	vsnprintf(str, len+1, format, ap);
	va_end(ap);

	return str;
}

bool
key_in_list(const char * const list[], const char * key)
{
	for (int i = 0; list && list[i]; i++)
	{
		if (strcmp(list[i], key) == 0)
			return true;
	}
	return false;
}

bool
string_has_suffix(const char * string, const char * suffix)
{
	if (strlen(string) < strlen(suffix))
		return false;

	int offset = strlen(string) - strlen(suffix);
	return (strcmp(string + offset, suffix) == 0);
}

void
free_array(char ** array)
{
	for (int i = 0; array && array[i]; i++)
	{
		free(array[i]);
	}
	free(array);
}
