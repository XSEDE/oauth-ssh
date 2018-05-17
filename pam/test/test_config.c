/*
 * Copyright The University of Chicago
 * All Rights Reserved.
 */

#define _BSD_SOURCE /* For mkstemp(3) */
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "../config.h"
#include "unit_test.h"


static void
test_config_init_free(void ** state)
{
	struct config * config;
	assert_non_null(config = config_init());
	config_free(config);
}

static void
test_config_section(void ** state)
{
	struct config * config = config_init();

	char ** sections;

	//
	// No sections initially
	//
	config_get_sections(config, &sections);
	assert_null(sections);

	assert_false(config_section_exists(config, "section1"));
	assert_false(config_section_exists(config, "section2"));
	assert_false(config_section_exists(config, "section3"));

	//
	// Single section
	//
	config_add_section(config, "section1");

	assert_true(config_section_exists(config, "section1"));
	assert_false(config_section_exists(config, "section2"));
	assert_false(config_section_exists(config, "section3"));

	config_get_sections(config, &sections);
	// Validate the returned array
	assert_non_null(sections);
	assert_non_null(sections[0]);
	assert_string_equal(sections[0], "section1");
	// Null terminated array
	assert_null(sections[1]);
	// Our responsibility to release sections
	test_free(sections[0]);
	test_free(sections);

	//
	// Duplicate section ignored
	//
	config_add_section(config, "section1");

	assert_true(config_section_exists(config, "section1"));
	assert_false(config_section_exists(config, "section2"));
	assert_false(config_section_exists(config, "section3"));

	config_get_sections(config, &sections);
	// Validate the returned array
	assert_non_null(sections);
	assert_non_null(sections[0]);
	assert_string_equal(sections[0], "section1");
	// Null terminated array
	assert_null(sections[1]);
	// Our responsibility to release sections
	test_free(sections[0]);
	test_free(sections);

	//
	// Two sections
	//
	config_add_section(config, "section2");

	assert_true(config_section_exists(config, "section1"));
	assert_true(config_section_exists(config, "section2"));
	assert_false(config_section_exists(config, "section3"));

	config_get_sections(config, &sections);
	// Validate the returned array
	assert_non_null(sections);
	assert_non_null(sections[0]);
	assert_non_null(sections[1]);
	assert_true(strcmp(sections[0], "section1") == 0 || 
	            strcmp(sections[1], "section1") == 0);
	assert_true(strcmp(sections[0], "section2") == 0 || 
	            strcmp(sections[1], "section2") == 0);
	// Null terminated array
	assert_null(sections[2]);
	// Our responsibility to release sections
	test_free(sections[0]);
	test_free(sections[1]);
	test_free(sections);

	//
	// Back to a single section
	//
	config_del_section(config, "section1");

	assert_false(config_section_exists(config, "section1"));
	assert_true(config_section_exists(config, "section2"));
	assert_false(config_section_exists(config, "section3"));

	config_get_sections(config, &sections);
	// Validate the returned array
	assert_non_null(sections);
	assert_non_null(sections[0]);
	assert_string_equal(sections[0], "section2");
	// Null terminated array
	assert_null(sections[1]);
	// Our responsibility to release sections
	test_free(sections[0]);
	test_free(sections);

	config_free(config);
}

static void
test_config_key(void ** state)
{
	struct config * config = config_init();

	//
	// No sections
	//

	// No section, key does not exist
	assert_false(config_key_exists(config, "section1", "key1"));

	// No section, no keys to get
	char ** keys;
	config_get_keys(config, "section1", &keys);
	assert_null(keys);

	// No section, no key, no value
	char * value;
	config_get_value(config, "section1", "key1", &value);
	assert_null(value);

	//
	// section1, empty
	//
	config_add_section(config, "section1");

	// Empty section, key does not exist
	assert_false(config_key_exists(config, "section1", "key1"));

	// Empty section, no keys to get
	config_get_keys(config, "section1", &keys);
	assert_null(keys);

	// Empty section, no key, no value
	config_get_value(config, "section1", "key1", &value);
	assert_null(value);

	//
	// section1 with 'key1' no value
	//
	config_add_key(config, "section1", "key1", NULL);

	// key exists
	assert_true(config_key_exists(config, "section1", "key1"));

	// We can get keys
	config_get_keys(config, "section1", &keys);
	assert_non_null(keys);
	assert_non_null(keys[0]);
	assert_null(keys[1]);
	assert_true(strcmp(keys[0], "key1") == 0);
	// Our responsibility to release 
	test_free(keys[0]);
	test_free(keys);

	// key1 has no value
	config_get_value(config, "section1", "key1", &value);
	assert_null(value);

	//
	// section1 with 'key2' with 'value2'
	//
	config_add_key(config, "section1", "key2", "value2");

	// key exists
	assert_true(config_key_exists(config, "section1", "key2"));

	// We can get keys
	config_get_keys(config, "section1", &keys);
	assert_non_null(keys);
	assert_non_null(keys[0]);
	assert_non_null(keys[1]);
	assert_null(keys[2]);
	assert_true(strcmp(keys[0], "key1") == 0 || strcmp(keys[1], "key1") == 0);
	assert_true(strcmp(keys[0], "key2") == 0 || strcmp(keys[1], "key2") == 0);
	// Our responsibility to release 
	test_free(keys[0]);
	test_free(keys[1]);
	test_free(keys);

	// key2 has value 'value2'
	config_get_value(config, "section1", "key2", &value);
	assert_non_null(value);
	assert_string_equal(value, "value2");
	test_free(value);

	//
	// auto-build section 2 by adding key 3
	//
	config_add_key(config, "section2", "key3", "value3");
	// key exists
	assert_true(config_key_exists(config, "section2", "key3"));

	config_free(config);
}

static void
test_config_load_save(void ** state)
{
	char * template = test_strdup("/tmp/config_test_XXXXXX");
	close(mkstemp(template));

	// Load an empty file
	struct config * config = config_init();
	assert_int_equal(config_load(config, template), 0);

	char ** sections;
	config_get_sections(config, &sections);
	assert_null(sections);
	config_free(config);

	//
	// Save a section and two keys, one with value, one without
	//
	config = config_init();
	config_add_section(config, "section1");
	config_add_key(config, "section1", "key1", NULL);
	config_add_key(config, "section1", "key2", "value2");
	assert_int_equal(config_save(config, template), 0);
	config_free(config);

	//
	// Load a section
	//
	config = config_init();
	assert_int_equal(config_load(config, template), 0);
	// Get the sections
	config_get_sections(config, &sections);
	assert_non_null(sections);
	assert_non_null(sections[0]);
	assert_null(sections[1]);
	assert_true(strcmp(sections[0], "section1") == 0);
	test_free(sections[0]);
	test_free(sections);

	// Get the keys
	char ** keys;
	config_get_keys(config, "section1", &keys);
	assert_non_null(keys);
	assert_non_null(keys[0]);
	assert_non_null(keys[1]);
	assert_null(keys[2]);
	assert_true(strcmp(keys[0], "key1") == 0 || strcmp(keys[1], "key1") == 0);
	assert_true(strcmp(keys[0], "key2") == 0 || strcmp(keys[1], "key2") == 0);
	test_free(keys[0]);
	test_free(keys[1]);
	test_free(keys);

	// Get the values
	char * value;
	config_get_value(config, "section1", "key1", &value);
	assert_null(value);
	config_get_value(config, "section1", "key2", &value);
	assert_non_null(value);
	assert_string_equal(value, "value2");
	test_free(value);

	config_free(config);

	unlink(template);
	test_free(template);
}

static void
test_config_load_error(void ** state)
{
	char * template = test_strdup("/tmp/config_test_XXXXXX");
	int fd = mkstemp(template);

	unlink(template);

	struct config * config;
	config = config_init();
	assert_int_equal(config_load(config, template), -ENOENT);

	test_free(template);
}

static void
test_is_empty_line(void ** state)
{
	int is_empty_line(const char * line);

	assert_true(is_empty_line(""));
	assert_true(is_empty_line("  "));
	assert_false(is_empty_line(" x"));
}

static void
test_is_comment(void ** state)
{
	int is_comment(const char * line);

	assert_true(is_comment("#"));
	assert_true(is_comment("  #  sdf"));
	assert_false(is_comment("    sdf"));
}


static void
test_is_section(void ** state)
{
	int is_section(const char * line, char ** section);

	char * section;
	assert_true(is_section("[section]", &section));
	assert_string_equal(section, "section");
	test_free(section);

	assert_true(is_section("  [  section  ] ", &section));
	assert_string_equal(section, "section");
	test_free(section);

	assert_false(is_section("  [ x section  ] ", &section));
	assert_false(is_section("  [   section   ", &section));
}

static void
test_is_key_value(void ** state)
{
	int is_key_value(const char * line, char ** key, char ** value);

	char * key;
	char * value;

	assert_true(is_key_value(" key value ", &key, &value));
	assert_string_equal(key, "key");
	assert_string_equal(value, "value");
	test_free(key);
	test_free(value);

	assert_true(is_key_value("key=value", &key, &value));
	assert_string_equal(key, "key=value");
	assert_null(value);
	test_free(key);

	assert_true(is_key_value(" key = ", &key, &value));
	assert_string_equal(key, "key");
	assert_string_equal(value, "=");
	test_free(key);
	test_free(value);

	assert_false(is_key_value(" key = value ", &key, &value));
}


int
main()
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_config_init_free),
		cmocka_unit_test(test_config_section),
		cmocka_unit_test(test_config_key),
		cmocka_unit_test(test_is_empty_line),
		cmocka_unit_test(test_is_comment),
		cmocka_unit_test(test_is_section),
		cmocka_unit_test(test_is_key_value),
		cmocka_unit_test(test_config_load_save),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}

