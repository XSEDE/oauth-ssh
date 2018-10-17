/*
 * Copyright The University of Chicago
 * All Rights Reserved.
 */

/*
 * Local includes.
 */
#include "unit_test.h"
#include "../introspect.c"
#include "../strings.c"
#include "../json.c"
#include "../credentials.c"

/*******************************************
 *              MOCKS
 *******************************************/
#include "mock_http.h"

/*******************************************
 *              TESTS
 *******************************************/
struct introspect I1 = {
	.active       = 1,
	.scopes       = (char *[]){"1", "2", NULL},
	.sub          = "SUB",
	.username     = "USERNAME",
	.display_name = "DISPLAY_NAME",
	.email        = "EMAIL",
	.client_id    = "CLIENT_ID",
	.audiences    = (char *[]){"AUD1", "AUD2", NULL},
	.issuer       = "ISSUER",
	.expiry       = 1234,
	.issued_at    = 5678,
	.not_before   = 9012,
	.identities   = (char *[]){"ID1", "ID2", NULL},
};

char * J1 = 
"{"
"\"active\"         : true,"
"\"scope\"          : \"1 2\","
"\"sub\"            : \"SUB\","
"\"username\"       : \"USERNAME\","
"\"name\"           : \"DISPLAY_NAME\","
"\"email\"          : \"EMAIL\","
"\"client_id\"      : \"CLIENT_ID\","
"\"aud\"            : \"AUD1 AUD2\","
"\"iss\"            : \"ISSUER\","
"\"exp\"            : 1234,"
"\"iat\"            : 5678,"
"\"nbf\"            : 9012,"
"\"identities_set\" : [\"ID1\", \"ID2\"]"
"}";

void
test_cast_valid_json(void ** state)
{
	struct http_mock m = {0, strdup(J1), NULL};
	will_return(http_post_request, &m);

	struct credentials creds = init_client_creds("ID", "SECRET");
	struct introspect * i = NULL;
	char * error_msg = NULL;
	int retval = introspect(&creds, "TOKEN", &i, &error_msg);
	assert_int_equal(retval, 0);

	assert_int_equal(I1.active, i->active);
	assert_string_equal(I1.scopes[0], i->scopes[0]);
	assert_string_equal(I1.scopes[1], i->scopes[1]);
	assert_string_equal(I1.sub, i->sub);
	assert_string_equal(I1.username, i->username);
	assert_string_equal(I1.display_name, i->display_name);
	assert_string_equal(I1.email, i->email);
	assert_string_equal(I1.client_id, i->client_id);
	assert_string_equal(I1.audiences[0], i->audiences[0]);
	assert_string_equal(I1.audiences[1], i->audiences[1]);
	assert_string_equal(I1.issuer, i->issuer);
	assert_int_equal(I1.expiry, i->expiry);
	assert_int_equal(I1.issued_at, i->issued_at);
	assert_int_equal(I1.not_before, i->not_before);
	assert_string_equal(I1.identities[0], i->identities[0]);
	assert_string_equal(I1.identities[1], i->identities[1]);

	free_introspect(i);
}


const char * J2 =
"{"
"   \"active\"   : true,"
"   \"scope\"    : \"urn:globus:auth:scope:service.example.com:all\","
"   \"client_id\": \"d430e6c8-b06f-4446-a060-2b6b2bc3e54a\","
"   \"sub\"      : \"2982f207-04c0-11e5-ac60-22000b92c6ec\","
"   \"username\" : \"user1@example.com\","
"   \"aud\"      : \"server.example.com\","
"   \"iss\"      : \"https://auth.globus.org/\","
"   \"exp\"      : 1419356238,"
"   \"iat\"      : 1419350238,"
"   \"nbf\"      : 1419350238,"
"   \"identities_set\": ["
"       \"2982f207-04c0-11e5-ac60-22000b92c6ec\","
"       \"3982f207-04c0-11e5-ac60-22000b92c6ed\""
"   ],"
"   \"name\"     : \"Joe User\","
"   \"email\"    : \"user1@example.dom\""
"}";

void
test_cast_web_example(void ** state)
{
	struct http_mock m = {0, strdup(J2), NULL};
	will_return(http_post_request, &m);

	struct credentials creds = init_client_creds("ID", "SECRET");
	struct introspect * i = NULL;
	char * error_msg = NULL;
	int retval = introspect(&creds, "TOKEN", &i, &error_msg);
	assert_int_equal(retval, 0);

	// active
	assert_true(i->active);
	// scope
	assert_string_equal(i->scopes[0], "urn:globus:auth:scope:service.example.com:all");
	assert_null(i->scopes[1]);
	// client id
	assert_string_equal(i->client_id, "d430e6c8-b06f-4446-a060-2b6b2bc3e54a");
	// sub
	assert_string_equal(i->sub, "2982f207-04c0-11e5-ac60-22000b92c6ec");
	// username
	assert_string_equal(i->username, "user1@example.com");
	// aud
	assert_string_equal(i->audiences[0], "server.example.com");
	assert_null(i->audiences[1]);
	// iss
	assert_string_equal(i->issuer, "https://auth.globus.org/");
	// exp
	assert_int_equal(i->expiry, 1419356238);
	// iat
	assert_int_equal(i->issued_at, 1419350238);
	// nbf
	assert_int_equal(i->not_before, 1419350238);
	// identities_set
	assert_string_equal(i->identities[0], "2982f207-04c0-11e5-ac60-22000b92c6ec");
	assert_string_equal(i->identities[1], "3982f207-04c0-11e5-ac60-22000b92c6ed");
	assert_null(i->identities[2]);
	// name
	assert_string_equal(i->display_name, "Joe User");
	// email
	assert_string_equal(i->email, "user1@example.dom");

	free_introspect(i);
}

/*******************************************
 *              FIXTURES
 *******************************************/

int
main()
{
	const struct CMUnitTest tests[] = {
		{"cast web JSON reply",   test_cast_web_example},
		{"cast valid JSON reply", test_cast_valid_json},
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}

