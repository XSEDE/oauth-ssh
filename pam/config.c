#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <regex.h>
#include <errno.h>
#include "config.h"

#ifdef UNIT_TEST
  #include "test/unit_test.h"
#endif

struct key_value {
	char * key;
	char * value;
	struct key_value * next_key_value;
};

struct section {
	char * name;
	struct key_value * key_values;
	struct section * next_section;
};

struct config {
	struct section * sections;
};

struct config *
config_init()
{
	return calloc(sizeof(struct config), 1);
}

void
config_free(struct config * config)
{
	while (config->sections)
		config_del_section(config, config->sections->name);
	free(config);
}

#define SPC "[[:space:]]*"
#define CHR "[^[:space:]]"

const char * COMMENT = "^" SPC "#";
const char * SECTION = "^" SPC "\\[" SPC "(" CHR"+" ")" SPC "\\]" SPC "$";
const char * KEYVAL  = "^" SPC "(" CHR"+" ")" SPC "(" CHR"*" ")" SPC "$";
const char * EMPTY   = "^" SPC "$";

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
is_section(const char * line, char ** section)
{
	regmatch_t pmatch[3];
	regex_t preg;

	*section = NULL;

	regcomp(&preg, SECTION, REG_EXTENDED);
	int retval = regexec(&preg, line, 3, pmatch, 0);
	if (retval == 0)
		*section = strndup(&line[pmatch[1].rm_so],
		                    pmatch[1].rm_eo-pmatch[1].rm_so);

	regfree(&preg);

	return (retval == 0);
}

static int
is_key_value(const char * line, char ** key, char ** value)
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

	return (retval == 0);
}

int
config_load(struct config * config, 
            const char    * path)
{
	FILE * f = fopen(path, "r");

	if (!f) return -errno;

	char buf[1024];

	char * current_section = NULL;
	char * next_section = NULL;
	char * key = NULL;
	char * value = NULL;

	int retval = 0;
	int lineno = 0;
	while (fgets(buf, sizeof(buf), f))
	{
		lineno++;

		if (buf[strlen(buf)-1] == '\n')
			buf[strlen(buf)-1] = '\0';

		if (is_comment(buf)) continue;
		if (is_empty_line(buf)) continue;

		if (is_section(buf, &next_section))
		{
			config_add_section(config, next_section);
			if (current_section)
				free(current_section);
			current_section = next_section;
			continue;
		}

		if (current_section && is_key_value(buf, &key, &value))
		{
			config_add_key(config, current_section, key, value);
			free(key);
			if (value) free(value);
			continue;
		}

		retval = lineno;
		break;
	}

cleanup:
	if (current_section)
		free(current_section);

	fclose(f);
	return 0;
}

int
config_save(const struct config * config,
            const char          * path)
{
	FILE * f = fopen(path, "w");

	for (struct section * section = config->sections; section; section = section->next_section)
	{
		fwrite("[", sizeof(char), 1, f);
		fwrite(section->name, sizeof(char), strlen(section->name), f);
		fwrite("]\n", sizeof(char), 2, f);

		for (struct key_value * key_value = section->key_values;
		     key_value; 
		     key_value = key_value->next_key_value)
		{
			fwrite(key_value->key, sizeof(char), strlen(key_value->key), f);
			fwrite(" ", sizeof(char), 1, f);
			if (key_value->value)
				fwrite(key_value->value, sizeof(char), strlen(key_value->value), f);
			fwrite("\n", sizeof(char), 1, f);
		}
	}

	fclose(f);
	return 0;
}

void
config_add_section(struct config * config,
                   const char    * section_name)
{
	if (config_section_exists(config, section_name))
		return;

	struct section * s = calloc(sizeof(struct section), 1);
	s->name = calloc(sizeof(char), strlen(section_name)+1);
	strcpy(s->name, section_name);

	s->next_section = config->sections;
	config->sections = s;
}

void
config_del_section(struct config * config,
                   const char    * section_name)
{
	if (!config_section_exists(config, section_name))
		return;

	struct section ** section_ptr = &config->sections;

	while (*section_ptr && strcmp((*section_ptr)->name, section_name) != 0)
	{
		section_ptr = &(*section_ptr)->next_section;
	}

	if (*section_ptr == NULL)
		return;

	while ((*section_ptr)->key_values)
		config_del_key(config, 
		               section_name, 
		               (*section_ptr)->key_values->key);

	struct section * section_to_free = *section_ptr;
	*section_ptr = (*section_ptr)->next_section;

	free(section_to_free->name);
	free(section_to_free);
}

void
config_get_sections(struct config *   config,
                    char          *** sections)
{
	*sections = NULL;

	int count = 0;
	struct section * section_ptr = config->sections;
	for (count = 0; section_ptr; section_ptr = section_ptr->next_section, count++);

	if (count == 0) return;

	*sections = calloc(count + 1, sizeof(char *));

	section_ptr = config->sections;
	for (int index = 0; section_ptr; section_ptr = section_ptr->next_section, index++)
	{
		(*sections)[index] = strdup(section_ptr->name);
	}
	
}

int
config_section_exists(struct config * config, 
                      const char    * section)
{
	for (struct section * section_ptr = config->sections; 
	     section_ptr; 
	     section_ptr = section_ptr->next_section)
	{
		if (strcmp(section_ptr->name, section) == 0)
			return 1;
	}

	return 0;
}

void
config_add_key(struct config * config, 
               const char    * section_name,
               const char    * key,
               const char    * value)
{
	if (!config_section_exists(config, section_name))
		config_add_section(config, section_name);

	struct section * section = config->sections;

	while (section && strcmp(section->name, section_name) != 0)
	{
		section = section->next_section;
	}

	struct key_value * key_value = section->key_values;
	while (key_value && strcmp(key_value->key, key) != 0)
	{
		key_value = key_value->next_key_value;
	}

	if (!key_value)
	{
		key_value = calloc(1, sizeof(struct key_value));
		key_value->key = strdup(key);
		key_value->next_key_value = section->key_values;
		section->key_values = key_value;
	}

	if (key_value->value)
	{
		free(key_value->value);
		key_value->value = NULL;
	}

	if (value)
		key_value->value = strdup(value);
}

void
config_del_key(struct config * config, 
               const char    * section_name, 
               const char    * key)
{
	struct section * section = config->sections;

	while (section && strcmp(section->name, section_name) != 0)
	{
		section = section->next_section;
	}

	if (!section)
		return;

	struct key_value ** key_value = &section->key_values;
	while (*key_value && strcmp((*key_value)->key, key) != 0)
	{
		*key_value = (*key_value)->next_key_value;
	}

	if (!*key_value)
		return;

	if ((*key_value)->value)
		free((*key_value)->value);
	free((*key_value)->key);
	struct key_value * key_value_to_free = *key_value;
	*key_value = (*key_value)->next_key_value;
	free(key_value_to_free);
}

void
config_get_keys(struct config *   config,
                const char    *   section_name,
                char          *** keys)
{
	*keys = NULL;

	struct section * section = config->sections;

	while (section && strcmp(section->name, section_name) != 0)
	{
		section = section->next_section;
	}

	if (!section || !section->key_values)
		return;

	int count = 0;
	for (struct key_value * key_value = section->key_values; 
	     key_value; 
	     key_value=key_value->next_key_value)
	{
		count++;
	}

	*keys = calloc(count+1, sizeof(char *));

	int index = 0;
	for (struct key_value * key_value = section->key_values; 
	     key_value; 
	     key_value=key_value->next_key_value)
	{
		(*keys)[index++] = strdup(key_value->key);
	}
}

int
config_key_exists(struct config * config,
                  const char    * section_name,
                  const char    * key_name)
{
	struct section * section = config->sections;

	while (section && strcmp(section->name, section_name) != 0)
	{
		section = section->next_section;
	}

	if (!section)
		return 0;

	for (struct key_value * key_value = section->key_values; 
	     key_value; 
	     key_value=key_value->next_key_value)
	{
		if (strcmp(key_name, key_value->key) == 0)
			return 1;
	}

	return 0;
}

void
config_get_value(struct config *  config,
                 const char    *  section_name,
                 const char    *  key_name,
                 char          ** value)
{
	*value = NULL;

	struct section * section = config->sections;

	while (section && strcmp(section->name, section_name) != 0)
	{
		section = section->next_section;
	}

	if (!section)
		return;

	for (struct key_value * key_value = section->key_values; 
	     key_value; 
	     key_value=key_value->next_key_value)
	{
		if (strcmp(key_name, key_value->key) == 0)
		{
			if (key_value->value)
				*value = strdup(key_value->value);
			return;
		}
	}
}
