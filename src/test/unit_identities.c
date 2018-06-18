/*
 * Copyright The University of Chicago
 * All Rights Reserved.
 */

/*
 * System includes.
 */
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>

/*
 * Local includes.
 */
#include "unit_test.h"
#include "../identities.c"
#include "../strings.c"
#include "../json.c"

/*******************************************
 *              MOCKS
 *******************************************/
#include "mock_http.h"

/*******************************************
 *              TESTS
 *******************************************/
#define UUID1 "4c1c0d6f-3235-4709-b08b-d52777d5a93a"
#define UUID2 "fa74f495-6ebf-4ef3-8c21-73c9e914e430"
#define UUID3 "309efd97-027e-4072-b673-3516f94823b5"
#define USER1 "johndoe@example.com"
#define USER2 "janedoe@example.com"
#define USER3 "root@example.com"

const struct identity ID1 = {UUID1,
                             USER1,
                             ID_STATUS_USED,
                             "IDP1",
                             "EMAIL1",
                             "NAME1",
                             "ORGANIZTION1"};

const struct identity ID2 = {UUID2,
                             USER2,
                             ID_STATUS_UNUSED,
                             "IDP2",
                             "EMAIL2",
                             "NAME2",
                             "ORGANIZTION2"};

const struct identity ID3 = {UUID3,
                             USER3,
                             ID_STATUS_PRIVATE,
                             "IDP3",
                             "EMAIL3",
                             "NAME3",
                             "ORGANIZTION3"};
/*
 * JSON helper functions.
 */
static int
idp_exists(struct json_object * j_arr, const char * uuid)
{
	for (int i = 0; i < json_object_array_length(j_arr); i++)
	{
		struct json_object * jobj = json_object_array_get_idx(j_arr, i);
		struct json_object * j_uuid;
		json_object_object_get_ex(jobj, "id", &j_uuid);

		if (strcmp(uuid, json_object_get_string(j_uuid)) == 0)
			return 1;
	}

	return 0;
}

static void
add_idp_section(const struct identity * identities[],
                struct json_object         * jobj)
{
	struct json_object * j_arr = json_object_new_array();

	for (int i = 0; identities[i]; i++)
	{
		if (!idp_exists(j_arr, identities[i]->identity_provider))
		{

			struct json_object * id_obj = json_object_new_object();

			const char * uuid = identities[i]->identity_provider;
			struct json_object * j_uuid = json_object_new_string(uuid);
			json_object_object_add(id_obj, "id", j_uuid);

			struct json_object * j_name = json_object_new_string("blahblah");
			json_object_object_add(id_obj, "name", j_name);

			json_object_array_add(j_arr, id_obj);
		}
	}

	struct json_object * j_inc = json_object_new_object();

	json_object_object_add(j_inc, "identity_providers", j_arr);
	json_object_object_add(jobj,  "included", j_inc);
}

static void
add_identities(const struct identity * identities[],
               struct json_object    * jobj)
{
	struct json_object * j_arr = json_object_new_array();

	for (int i = 0; identities[i]; i++)
	{
		struct json_object * j_id = json_object_new_object();

		const char * string = identities[i]->username;
		struct json_object * j_tmp = json_object_new_string(string);
		json_object_object_add(j_id, "username", j_tmp);

		string = identities[i]->name;
		j_tmp = json_object_new_string(string);
		json_object_object_add(j_id, "name", j_tmp);

		string = identities[i]->id;
		j_tmp = json_object_new_string(string);
		json_object_object_add(j_id, "id", j_tmp);

		string = identities[i]->identity_provider;
		j_tmp = json_object_new_string(string);
		json_object_object_add(j_id, "identity_provider", j_tmp);

		string = identities[i]->organization;
		j_tmp = json_object_new_string(string);
		json_object_object_add(j_id, "organization", j_tmp);

		string = identities[i]->email;
		j_tmp = json_object_new_string(string);
		json_object_object_add(j_id, "email", j_tmp);

		switch (identities[i]->status)
		{
		case ID_STATUS_UNUSED:
			string = "unused";
			break;
		case ID_STATUS_USED:
			string = "used";
			break;
		case ID_STATUS_PRIVATE:
			string = "private";
			break;
		case ID_STATUS_CLOSED:
			string = "closed";
			break;
		}

		j_tmp = json_object_new_string(string);
		json_object_object_add(j_id, "status", j_tmp);

		json_object_array_add(j_arr, j_id);
	}

	json_object_object_add(jobj, "identities", j_arr);
}

static char *
identities_to_json_text(const struct identity * identities[])
{
	struct json_object * jobj = json_object_new_object();

	if (identities && identities[0])
	{
		add_idp_section(identities, jobj);
		add_identities(identities, jobj);
	}

	const char * tmp = json_object_to_json_string_ext(jobj,
	                                                  JSON_C_TO_STRING_PRETTY|
	                                                  JSON_C_TO_STRING_SPACED);
	char * text = strdup(tmp);
	json_object_put(jobj);

	return text;
}

int
strings_equal(const char * s1, const char * s2)
{
	if ((s1 == NULL) ^ (s2 == NULL))
		return 0;

	if (s1 && strcmp(s1, s2))
		return 0;

	return 1;
}

static int
identities_equal(const struct identity * identity1,
                 const struct identity * identity2)
{
	if (!strings_equal(identity1->id, identity2->id))
		return 0;

	if (!strings_equal(identity1->username, identity2->username))
		return 0;

	if (!strings_equal(identity1->identity_provider,
	                   identity2->identity_provider))
	{
		return 0;
	}

	if (!strings_equal(identity1->name, identity2->name))
		return 0;

	if (!strings_equal(identity1->organization, identity2->organization))
		return 0;

	if (identity1->status != identity2->status)
		return 0;

	return 1;
}

/*
 * Helper tests
 */

static void
test_is_uuid(void ** state)
{
	assert_true(is_uuid(UUID1));
	assert_false(is_uuid(USER1));
}

static void
test_is_username(void ** state)
{
	assert_true(is_username(USER1));
	assert_false(is_username(UUID1));
}

static void
test_build_uuid_request(void ** state)
{
	char * request = NULL;

	request = build_uuid_request((const char *[]){NULL});
	assert_null(request);

	request = build_uuid_request((const char *[]){UUID1, NULL});
	assert_string_equal(request, "ids=" UUID1);
	free(request);

	request = build_uuid_request((const char *[]){UUID1, UUID2, NULL});
	assert_string_equal(request, "ids=" UUID1 "," UUID2);
	free(request);

	request = build_uuid_request((const char *[]){UUID1, USER1, NULL});
	assert_string_equal(request, "ids=" UUID1);
	free(request);

	request = build_uuid_request((const char *[]){USER1, NULL});
	assert_null(request);
}

static void
test_build_username_request(void ** state)
{
	char * request = NULL;

	request = build_username_request((const char *[]){NULL});
	assert_null(request);

	request = build_username_request((const char *[]){USER1, NULL});
	assert_string_equal(request, "usernames=" USER1);
	free(request);

	request = build_username_request((const char *[]){USER1, USER2, NULL});
	assert_string_equal(request, "usernames=" USER1 "," USER2);
	free(request);

	request = build_username_request((const char *[]){USER1, UUID1, NULL});
	assert_string_equal(request, "usernames=" USER1);
	free(request);

	request = build_username_request((const char *[]){UUID1, NULL});
	assert_null(request);
}

struct find_unique_ids_cb {
	int count;
};

static void
find_unique_ids_cb(json_t * json, void * cb_arg)
{
	struct find_unique_ids_cb * arg = cb_arg;
	arg->count++;
}

static void
test_find_unique_ids(void ** state)
{
	char * errmsg = NULL;

	const struct identity * ids1[] = {&ID1, &ID2, NULL};
	char   * json_text1 = identities_to_json_text(ids1);
	json_t * json1 = json_init(json_text1, &errmsg);

	const struct identity * ids2[] = {&ID2, &ID3, NULL};
	char   * json_text2 = identities_to_json_text(ids2);
	json_t * json2 = json_init(json_text2, &errmsg);

	void (*f)(json_t * json, void * cb_arg) = find_unique_ids_cb;
	struct find_unique_ids_cb arg = {0};

	memset(&arg, 0, sizeof(arg));
	find_unique_ids(NULL,  NULL,  f, &arg);
	assert_int_equal(arg.count,  0);

	memset(&arg, 0, sizeof(arg));
	find_unique_ids(json1, NULL,  f, &arg);
	assert_int_equal(arg.count,  2);

	memset(&arg, 0, sizeof(arg));
	find_unique_ids(NULL,  json2, f, &arg);
	assert_int_equal(arg.count,  2);

	memset(&arg, 0, sizeof(arg));
	find_unique_ids(json1, json2, f, &arg);
	assert_int_equal(arg.count,  3);

	free(json_text1);
	free(json_text2);
	json_free(json1);
	json_free(json2);
}

static void
test_json_to_identity(void ** state)
{
	char * errmsg = NULL;

	const struct identity * identities[] = {&ID1, NULL};
	char   * json_text = identities_to_json_text(identities);
	json_t * json = json_init(json_text, &errmsg);
	json_t * jarr = json_get_object(json, "identities");
	json_t * jidx = json_array_idx(jarr, 0);

	struct identity * identity = json_to_identity(jidx);

	assert_true(identities_equal(identity, &ID1));

	free_identity(identity);
	free(json_text);
	json_free(jidx);
	json_free(jarr);
	json_free(json);
}

static void
test_parse_json_replies(void ** state)
{
	struct identity ** identities_out;
	char * errmsg = NULL;

	const struct identity * identities_in[] = {&ID1, NULL};
	char   * jtext1 = identities_to_json_text(identities_in);
	json_t * json   = json_init(jtext1, &errmsg);
	free(jtext1);

	parse_json_replies(json, NULL, &identities_out, &errmsg);
	assert_true(identities_equal(identities_out[0], identities_in[0]));
	free_identities(identities_out);
	json_free(json);
}

static void
test_perform_request(void ** state)
{
	json_t * json_reply_body = NULL;
	char   * error_msg = NULL;

	struct http_mock m = {0, strdup("{\"key\" : \"value\"}"), NULL};
	will_return(http_get_request, &m);

	int retval = perform_request(NULL, NULL, &json_reply_body, &error_msg);
	assert_int_equal(retval, m.return_value);

	const char * string = json_get_string(json_reply_body, "key");
	assert_string_equal(string, "value");
	json_free(json_reply_body);
}

/*
 * Behavioral tests for our API.
 */
static void
test_http_error(void ** state)
{
	struct http_mock m = {1, NULL, strdup("Stuff is broken")};
	will_return(http_get_request, &m);

	struct identity ** identities;
	char * error_msg = NULL;

	int retval = get_identities(NULL,
	                           (const char *[]){UUID1, NULL},
	                           &identities,
	                           &error_msg);

	assert_true(retval != 0);
	assert_non_null(error_msg);
	free(error_msg);
}

static void
test_json_error(void ** state)
{
	struct http_mock m = {0, strdup("{BrokenJson]"), NULL};
	will_return(http_get_request, &m);

	struct identity ** identities;
	char * error_msg = NULL;

	int retval = get_identities(NULL,
	                            (const char *[]){USER1, NULL},
	                            &identities,
	                            &error_msg);

	assert_true(retval != 0);
	assert_non_null(error_msg);
	free(error_msg);
}

const char * json_reply_uuid = 
"{"
"  \"included\": {"
"    \"identity_providers\": ["
"      {"
"        \"id\": \"41143743-f3c8-4d60-bbdb-eeecaba85bd9\","
"        \"name\": \"Globus ID\""
"      }"
"    ]"
"  },"
"  \"identities\": ["
"    {"
"      \"username\": \"webapptester1@globusid.org\","
"      \"status\": \"used\","
"      \"name\": \"Jane Tester\","
"      \"id\": \"e9873f94-032a-11e6-afde-cb613ccc97a9\","
"      \"identity_provider\": \"41143743-f3c8-4d60-bbdb-eeecaba85bd9\","
"      \"organization\": null,"
"      \"email\": \"webapptester1@example.com\""
"    }"
"  ]"
"}";

static void
test_lookup_uuid(void ** state)
{
	struct http_mock m = {0, strdup(json_reply_uuid), NULL};
	will_return(http_get_request, &m);

	struct identity ** identities;
	char * error_msg;

	const char * uuids[] = {"e9873f94-032a-11e6-afde-cb613ccc97a9", NULL};

	int retval = get_identities(NULL, uuids, &identities, &error_msg);

	assert_true(retval == 0);

	assert_string_equal(identities[0]->id,
	                    "e9873f94-032a-11e6-afde-cb613ccc97a9");
	assert_string_equal(identities[0]->username,
	                    "webapptester1@globusid.org");
	assert_string_equal(identities[0]->identity_provider,
	                    "41143743-f3c8-4d60-bbdb-eeecaba85bd9");
	assert_int_equal(identities[0]->status, ID_STATUS_USED);
	assert_string_equal(identities[0]->email, "webapptester1@example.com");
	assert_string_equal(identities[0]->name, "Jane Tester");
	assert_null(identities[0]->organization);
	assert_null(identities[1]);

	free_identities(identities);
}

const char * json_reply_username = 
"{"
"  \"included\": {"
"    \"identity_providers\": ["
"      {"
"        \"id\": \"41143743-f3c8-4d60-bbdb-eeecaba85bd9\","
"        \"name\": \"Globus ID\""
"      }"
"    ]"
"  },"
"  \"identities\": ["
"    {"
"     \"username\": \"webapptester2@globusid.org\","
"     \"status\": \"private\","
"     \"name\": null,"
"     \"id\": \"e987941c-032a-11e6-afdf-7b65304db5f1\","
"     \"identity_provider\": \"41143743-f3c8-4d60-bbdb-eeecaba85bd9\","
"     \"organization\": null,"
"     \"email\": null"
"    }"
"  ]"
"}";

static void
test_lookup_username(void ** state)
{
	struct http_mock m = {0, strdup(json_reply_username), NULL};
	will_return(http_get_request, &m);

	struct identity ** identities;
	char * error_msg;

	const char * ids[] = {"webapptester2@globusid.org", NULL};
	int retval = get_identities(NULL, ids, &identities, &error_msg);

	assert_true(retval == 0);

	assert_string_equal(identities[0]->id,
	                    "e987941c-032a-11e6-afdf-7b65304db5f1");
	assert_string_equal(identities[0]->username, "webapptester2@globusid.org");
	assert_string_equal(identities[0]->identity_provider,
	                    "41143743-f3c8-4d60-bbdb-eeecaba85bd9");
	assert_int_equal(identities[0]->status, ID_STATUS_PRIVATE);
	assert_null(identities[0]->email);
	assert_null(identities[0]->name);
	assert_null(identities[0]->organization);
	assert_null(identities[1]);

	free_identities(identities);
}

/*******************************************
 *              FIXTURES
 *******************************************/

int
main()
{
	const struct CMUnitTest tests[] = {
		// Helper tests
// move to utilities.c
		{"is_uuid()", test_is_uuid},
		{"is_username()", test_is_username},

		{"build_uuid_request()", test_build_uuid_request},
		{"build_username_request()", test_build_username_request},
		{"test_find_unique_ids", test_find_unique_ids},

		{"json_to_auth_id()", test_json_to_identity},
		{"parse_json_replies()", test_parse_json_replies},
		{"perform_request()", test_perform_request},

		// Behavioral tests for our API.
		{"Return error from HTTP", test_http_error},
		{"Return error from JSON", test_json_error},
		{"Lookup by username", test_lookup_uuid},
		{"Lookup by uuid", test_lookup_username},
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}

