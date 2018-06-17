/*
 * System includes.
 */
#include <stdlib.h>
#include <string.h>
#include <regex.h>

/*
 * Local includes.
 */
#include "scope.h"

// XXX is it a mistake to assume that we would only get asked if we have
// the scope if the are able to introspect the scope and therefore it was
// intendend for this service so we don't need to check the fqdn?
int
has_scope(const char * suffix, char * list_of_scopes[], char ** errmsg)
{
	int found = 0;
	int error = 0;

	for (int i = 0; list_of_scopes[i] && !error && !found; i++)
	{
		const char * scope = list_of_scopes[i];
		regmatch_t match[SUFFIX_INDEX+1];
		regex_t reg;

		error = regcomp(&reg, SCOPE_REGEX, REG_EXTENDED);
		if (!error && regexec(&reg, scope, SUFFIX_INDEX+1, match, 0) == 0)
		{
			if (match[SUFFIX_INDEX].rm_so != -1)
			{
				if (strcmp(suffix, &scope[match[SUFFIX_INDEX].rm_so]) == 0)
					found = 1;
			}
		}

		if (error)
		{
			int len = regerror(error, &reg, NULL, 0);
			*errmsg = calloc(len, sizeof(char));
			regerror(error, &reg, *errmsg, len);
		}

		regfree(&reg);
	}
	return found;
}
