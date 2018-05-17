#include <stdlib.h>
#include <string.h>
#include "unit_test.h"

char *
test_strdup(const char * s)
{
	char * s1 = test_calloc(strlen(s)+1, sizeof(char));
	strcpy(s1, s);
	return s1;
}

char *
test_strndup(const char * s, size_t n)
{
	char * s1 = test_calloc(n+1, sizeof(char));
	strncpy(s1, s, n);
	return s1;
}
