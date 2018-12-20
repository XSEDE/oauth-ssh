/*
 * System includes.
 */
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/*
 * Local includes.
 */
#include "parser.h"
#include "debug.h" // always last

/*******************************************************************************
 * Internal (Private) Functions
 ******************************************************************************/

#define PARSER_READ_LEN 128

/*
 * Return the next non-empty line.
 */
static char *
_read_full_line(FILE * fptr)
{
	size_t length = PARSER_READ_LEN;
	char * buffer = calloc(length, sizeof(char));

	int eof = 0;
	int eol = 0;

	while (!eol && !eof)
	{
		if ((length - strlen(buffer)) < PARSER_READ_LEN)
		{
			length += PARSER_READ_LEN;
			buffer = realloc(buffer, length*sizeof(char));
		}

		eof = (fgets(&buffer[strlen(buffer)], PARSER_READ_LEN, fptr) == NULL);
		eol = (strchr(buffer, '\n') != NULL);
	}

	if (eof && strlen(buffer) == 0)
	{
		free(buffer);
		return NULL;
	}

	if (eol)
		*strchr(buffer, '\n') = '\0';

	return buffer;
}

/*
 * Return the next non-empty line sans comments.
 */
static char *
_read_line(FILE * fptr)
{
	char * line = NULL;

	while ((line = _read_full_line(fptr)) != NULL)
	{
		char * comment = strchr(line, '#');
		if (comment)
			*comment = '\0';

		for (int i = 0; line[i] != '\0'; i++)
		{
			if (!isspace(line[i]))
				return line;
		}
		free(line);
	}
	return NULL;
}

/*******************************************************************************
 * Public Functions
 ******************************************************************************/

bool
read_next_pair(FILE * fptr, char ** key, char *** values)
{
	*key = NULL;
	*values = NULL;

	char * line = _read_line(fptr);
	if (!line)
		return false;

	int index = 0;
	char * saveptr = NULL;
	char * token;

	while (true)
	{
		token = strtok_r(*key?NULL:line, " ,", &saveptr);
		if (!token)
			break;

		if (!*key)
		{
			*key = strdup(token);
			continue;
		}

		*values = realloc(*values, (index+2)*sizeof(char *));
		(*values)[index] = strdup(token);
		(*values)[++index] = NULL;
	}
	free(line);
	return true;
}
