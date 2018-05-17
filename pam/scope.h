#ifndef _SCOPE_H_
#define _SCOPE_H_

#define SCOPE_PREFIX "https://auth.globus.org/scopes/"

//A host name (label) can start or end with a letter or a number
//A host name (label) MUST NOT start or end with a '-' (dash)
//A host name (label) MUST NOT consist of all numeric values
//A host name (label) can be up to 63 characters

#define LABEL_REGEX "([[:alnum:]]+[\\-]*)*[[:alpha:]]+([\\-]*[[:alnum:]]+)*"

//#define LABEL_REGEX "[[:alnum:]]{1}([\\-]*[[:alnum:]]+)*"

#define FQDN_REGEX  "(" LABEL_REGEX "\\.){2,}"LABEL_REGEX

#define MIDFIX_REGEX FQDN_REGEX

#define SUFFIX_REGEX "[a-z0-9_]+"
#define SUFFIX_INDEX 6

#define SCOPE_REGEX SCOPE_PREFIX MIDFIX_REGEX "/" "("SUFFIX_REGEX")"

struct retval {
	char * error;
	int    found;
};

struct retval
has_scope(const char * suffix, const char * list_of_scopes[]);

#endif /* _SCOPE_H_ */
