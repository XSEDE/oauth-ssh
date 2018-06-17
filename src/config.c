/*
 * System includes.
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <regex.h>
#include <errno.h>

/*
 * Local includes.
 */
#include "config.h"

struct key_value {
	char * key;
	char * value;
	struct key_value * next;
};

struct config {
	struct key_value * key_values;
};

struct config *
config_init()
{
	return calloc(1, sizeof(struct config));
}

void
config_free(struct config * config)
{
	while (config->key_values)
	{
		struct key_value * tmp_kvs = config->key_values;
		config->key_values = config->key_values->next;
		free(tmp_kvs->key);
		if (tmp_kvs->value)
			free(tmp_kvs->value);
		free(tmp_kvs);
	}
	free(config);
}

#define SPC "[[:space:]]*"
#define CHR "[^[:space:]]"

const char * COMMENT = "^" SPC "#";
const char * KEYVAL  = "^" SPC "(" CHR"+" ")" SPC "(" CHR"*" ")" SPC "$";
const char * EMPTY   = "^" SPC "$";

static int
is_comment(const char * line)
{
	regmatch_t pmatch[3];
	regex_t preg;

	regcomp(&preg, COMMENT, REG_EXTENDED);
	int retval = regexec(&preg, line, 3, pmatch, 0);
	regfree(&preg);

	return (retval == 0);
}

static int
is_empty_line(const char * line)
{
	regmatch_t pmatch[3];
	regex_t preg;

	regcomp(&preg, EMPTY, REG_EXTENDED);
	int retval = regexec(&preg, line, 3, pmatch, 0);
	regfree(&preg);

	return (retval == 0);
}

static int
get_key_value(const char * line, char ** key, char ** value)
{
	regmatch_t pmatch[3];
	regex_t preg;

	*key = NULL;
	*value = NULL;

	regcomp(&preg, KEYVAL, REG_EXTENDED);
	int retval = regexec(&preg, line, 3, pmatch, 0);

	if (retval == 0)
	{
		*key = strndup(&line[pmatch[1].rm_so], pmatch[1].rm_eo-pmatch[1].rm_so);
		if (pmatch[2].rm_so != strlen(line))
			*value = strndup(&line[pmatch[2].rm_so],
			                  pmatch[2].rm_eo-pmatch[2].rm_so);
	}

	regfree(&preg);

	return retval;
}

int
config_load(struct config * config, 
            const char    * path)
{
	FILE * f = fopen(path, "r");

	if (!f) return -errno;

	char buf[1024];

	char * key = NULL;
	char * value = NULL;

	while (fgets(buf, sizeof(buf), f))
	{
		if (buf[strlen(buf)-1] == '\n')
			buf[strlen(buf)-1] = '\0';

		if (is_comment(buf)) continue;
		if (is_empty_line(buf)) continue;

		if (get_key_value(buf, &key, &value) == 0)
		{
			struct key_value * key_value = calloc(1, sizeof(*key_value));
			key_value->key = key;
			key_value->value = value;
			key_value->next = config->key_values;
			config->key_values = key_value;
		}
	}

	fclose(f);
	return 0;
}

void
config_get_value(struct config *  config,
                 const char    *  key_name,
                 char          ** value)
{
	*value = NULL;

	for (struct key_value * key_value = config->key_values;
	                        key_value;
	                        key_value = key_value->next)
	{
		if (strcmp(key_name, key_value->key) == 0)
		{
			if (key_value->value)
				*value = strdup(key_value->value);
			return;
		}
	}
}
