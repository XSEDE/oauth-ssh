#include <stdlib.h>
#include <string.h>
#include <regex.h>

#include "scope.h"

struct retval
has_scope(const char * suffix, const char * list_of_scopes[])
{
	struct retval retval = {NULL, 0};

	for (int i = 0; list_of_scopes[i] && !retval.error && !retval.found; i++)
	{
		const char * scope = list_of_scopes[i];
		regmatch_t match[SUFFIX_INDEX+1];
		regex_t reg;

		int error = regcomp(&reg, SCOPE_REGEX, REG_EXTENDED);
		if (!error && regexec(&reg, scope, SUFFIX_INDEX+1, match, 0) == 0)
		{
			if (match[SUFFIX_INDEX].rm_so != -1)
			{
				if (strcmp(suffix, &scope[match[SUFFIX_INDEX].rm_so]) == 0)
					retval.found = 1;
			}
		}

		if (error)
		{
			int len = regerror(error, &reg, NULL, 0);
			retval.error = calloc(len, sizeof(char));
			regerror(error, &reg, retval.error, len);
		}

		regfree(&reg);
	}
	return retval;
}
