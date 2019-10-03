/*
 * System includes.
 */
#include <syslog.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

/*
 * Local includes.
 */
#include "account_map.h"
#include "debug.h" // always last

/*******************************************
 *              MOCKS
 *******************************************/

// prevents our test from generating syslog messages
void vsyslog(int priority, const char *format, va_list ap) {}

#define MAGIC 0xDEADBEEF

struct file {
	int     magic;
	int     lineno;
	char ** contents;
};

FILE *
fopen(const char *path, const char *mode)
{
	struct file * f = (struct file *)path;
	if (f->magic == MAGIC)
		return (FILE *)path;
	return NULL;
}

char *
fgets(char *s, int size, FILE *stream)
{
	struct file * f = (struct file *)stream;

	if (!f->contents[f->lineno])
		return NULL;

	strcpy(s, f->contents[f->lineno++]);
	return s;
}

int
fclose(FILE *fp)
{
	return 0;
}

/*******************************************
 *              TESTS
 *******************************************/

struct identities identities =
	{
		{
			(struct identity_provider *[]) {
				&(struct identity_provider){
					.name = "41143743-f3c8-4d60-bbdb-eeecaba85bd9",
					.id = "Globus ID"
				},
				&(struct identity_provider){
					.name = "9e484f7f-a54f-44cb-a8c6-a1f57462b863",
					.id = "Example ID"
				},
				NULL
			}
		},

		(struct identity *[]){
			&(struct identity) {
				"jane@globusid.org",
				"used",
				//"Jane Doe",
				"e9873f94-032a-11e6-afde-cb613ccc97a9",
				"41143743-f3c8-4d60-bbdb-eeecaba85bd9",
				//"Globus",
				//"janedoe@globusid.org"
			},
			&(struct identity) {
				"john@example.com",
				"used",
				//"John Doe",
				"da0fcfb8-1231-4999-9b99-627f58f609c7",
				"9e484f7f-a54f-44cb-a8c6-a1f57462b863",
				//"Example Company",
				//"johndoe@example.com"
			},
			&(struct identity) {
				"doe@abc.com@accounts.google.org",
				"used",
				"2dfa2043-00ad-43b6-b93b-4c39baa00bdd",
				"9e484f7f-a54f-44cb-a8c6-a1f57462b863"
			},
			NULL
		}
	};

void
test_no_map_if_no_config(void ** state)
{
	struct config config = {.idp_suffix = NULL, .map_files = NULL};
	assert_null(account_map_init(&config, &identities));
}

void
test_no_map_if_no_suffix_match(void ** state)
{
	struct config config = {.idp_suffix = "abc.net", .map_files = NULL};
	assert_null(account_map_init(&config, &identities));
}

void
test_suffix_match_globusid(void ** state)
{
	struct config config = {.idp_suffix = "globusid.org", .map_files = NULL};
	struct account_map * map = account_map_init(&config, &identities);

	assert_string_equal(map->username, "jane@globusid.org");
	assert_string_equal(map->id, "e9873f94-032a-11e6-afde-cb613ccc97a9");
	assert_string_equal(map->accounts[0], "jane");
	assert_null(map->accounts[1]);
	assert_null(map->next);
	account_map_fini(map);
}

void
test_suffix_match_google(void ** state)
{
	struct config config = {.idp_suffix = "abc.com@accounts.google.org", .map_files = NULL};
	struct account_map * map = account_map_init(&config, &identities);
	assert_true(is_acct_in_map(map, "doe"));
	assert_false(is_acct_in_map(map, "doe@abc.com"));
	account_map_fini(map);

	config = (struct config){.idp_suffix = "accounts.google.org", .map_files = NULL};
	map = account_map_init(&config, &identities);
	assert_false(is_acct_in_map(map, "doe"));
	assert_false(is_acct_in_map(map, "doe@abc.com"));
	account_map_fini(map);
}

void
test_no_map_if_no_map_file(void ** state)
{
	struct config config = {
		.map_files = (char *[]){
			(char *) &(struct file) {
				MAGIC,
				0,
				(char *[]){NULL}
			},
			NULL
		}
	};
	assert_null(account_map_init(&config, &identities));
}

void
test_id_match_in_map_file(void ** state)
{
	struct config config = {
		.map_files = (char *[]){
			(char *) &(struct file) {
				MAGIC,
				0,
				(char *[]){"e9873f94-032a-11e6-afde-cb613ccc97a9 acct", NULL}
			},
			NULL
		}
	};

	struct account_map * map = account_map_init(&config, &identities);
	assert_string_equal(map->accounts[0], "acct");
	account_map_fini(map);
}

void
test_username_match_in_map_file(void ** state)
{
	struct config config = {
		.map_files = (char *[]){
			(char *) &(struct file) {
				MAGIC,
				0,
				(char *[]){"john@example.com acct", NULL}
			},
			NULL
		}
	};

	struct account_map * map = account_map_init(&config, &identities);
	assert_string_equal(map->accounts[0], "acct");
	account_map_fini(map);
}

//void
//test_add_acct_null_map(void ** state)
//{
//	struct account_map * map = NULL;
//	struct identity identity = {.username = "username", .id = "id"};
//
//	add_acct_mapping(&map, &identity, "acct");
//	assert_string_equal(map->username, "username");
//	assert_string_equal(map->id, "id");
//	assert_string_equal(map->accounts[0], "acct");
//	account_map_fini(map);
//}

//void
//test_add_acct_wo_matching_map(void ** state)
//{
//	struct account_map * map = calloc(1, sizeof(*map));
//	map->username = strdup("username");
//	map->id = strdup("id");
//	map->accounts = calloc(2, sizeof(char *));
//	map->accounts[0] = strdup("acct1");
//
//	struct identity identity = {.username = "username", .id = "id"};
//
//	add_acct_mapping(&map, &identity, "acct2");
//	assert_string_equal(map->username, "username");
//	assert_string_equal(map->id, "id");
//	assert_string_equal(map->accounts[1], "acct2");
//	account_map_fini(map);
//}

//void
//test_add_acct_to_existing_map(void ** state)
//{
//	struct account_map * map = NULL;
//
//	struct identity identity = {.username = "username", .id = "id"};
//
//	add_acct_mapping(&map, &identity, "acct");
//	assert_string_equal(map->username, "username");
//	assert_string_equal(map->id, "id");
//	assert_string_equal(map->accounts[0], "acct");
//	account_map_fini(map);
//}

//void
//test_add_acct_not_unique(void ** state)
//{
//	struct account_map * map = calloc(1, sizeof(*map));
//	map->username = strdup("username");
//	map->id = strdup("id");
//	map->accounts = calloc(2, sizeof(char *));
//	map->accounts[0] = strdup("acct");
//
//	struct identity identity = {.username = "username", .id = "id"};
//
//	add_acct_mapping(&map, &identity, "acct");
//	assert_string_equal(map->username, "username");
//	assert_string_equal(map->id, "id");
//	assert_string_equal(map->accounts[0], "acct");
//	account_map_fini(map);
//}

//void
//test_acct_from_username(void ** state)
//{
//	char * acct = acct_from_username("john@example.com");
//	assert_string_equal(acct, "john");
//	free(acct);
//
//	acct = acct_from_username("john@example.com@google.com");
//	assert_string_equal(acct, "john@example.com");
//	free(acct);
//}

//void
//test_suffix_match(void ** state)
//{
//	assert_true(id_matches_suffix(identities.identities[1], "example.com"));
//}

//void
//test_suffix_mismatch(void ** state)
//{
//	assert_false(id_matches_suffix(identities.identities[1], "abc.com"));
//}

void
test_acct_not_in_null_map(void ** state)
{
	assert_false(is_acct_in_map(NULL, "acct"));
}

void
test_acct_not_in_map(void ** state)
{
	struct account_map map = {
		.username = "username",
		.id = "id",
		.accounts = (char *[]){"acct1", NULL},
		.next = NULL
	};

	assert_false(is_acct_in_map(&map, "acct2"));
}

void
test_acct_in_map(void ** state)
{
	struct account_map map = {
		.username = "username",
		.id = "id",
		.accounts = (char *[]){"acct", NULL},
		.next = NULL
	};
	assert_true(is_acct_in_map(&map, "acct"));
}

int
main()
{
	const struct CMUnitTest tests[] = {
		{"no map if no config",            test_no_map_if_no_config},
		{"no map if no suffix match",      test_no_map_if_no_suffix_match},
		{"map on suffix match (globusid)", test_suffix_match_globusid},
		{"map on suffix match (google)",   test_suffix_match_google},
		{"no map if no map file",          test_no_map_if_no_map_file},
		{"id match in map file",           test_id_match_in_map_file},
		{"username match in map file",     test_username_match_in_map_file},
//		{"add acct to null map",           test_add_acct_null_map},
//		{"add acct w/o matching map",      test_add_acct_wo_matching_map},
//		{"add acct to existing map",       test_add_acct_to_existing_map},
//		{"add acct not unique",            test_add_acct_not_unique},
//		{"acct from username",             test_acct_from_username},
//		{"suffix match",                   test_suffix_match},
//		{"suffix mismatch",                test_suffix_mismatch},
		{"acct not in null map",           test_acct_not_in_null_map},
		{"acct not in map",                test_acct_not_in_map},
		{"acct in map",                    test_acct_in_map},
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}

//struct account_map *
//account_map_init(const struct config *, const struct identities *);

//void
//account_map_fini(struct account_map *);

//bool
//is_acct_in_map(const struct account_map * map, const char * acct);

//const char *
//acct_to_username(const struct account_map * map, const char * acct);

