/*
 * System includes.
 */
#include <syslog.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

/*
 * Local includes.
 */
#include "introspect.h"
#include "logger.h"
#include "json.h"
#include "debug.h" // always last

/*
 * These are the required fields in the introspect document. Tests
 * can modify this and pass it to create_json_string() to create a
 * JSON-like string suitable for create_introspect_doc().
 */
const char * minimum_required_fields[] =
{
	"  'active': true,           ",
	"  'scope': 'scopes',        ",
	"  'client_id': 'client_id', ",
	"  'sub': 'sub',             ",
	"  'username': 'username',   ",
	"  'aud': ['aud'],           ",
	"  'iss': 'iss',             ",
	"  'exp': 1,                 ",
	"  'iat': 2,                 ",
	"  'nbf': 3,                 ",
	"  'email': 'email',         ",
    // Additional null termination that can be replaced with a string value by
    // tests
	NULL, // Other
	NULL  // null terminated to end parsing
};

// These serve as indicies to minimum_required_fields so that tests can find the
// field they with to modify.
enum {
	ACTIVE = 0,
	SCOPE,
	CLIENT_ID,
	SUB,
	USERNAME,
	AUD,
	ISS,
	EXP,
	IAT,
	NBF,
	EMAIL,
	OTHER, // unit tests can add more fields here
};

// These allow us to convert the enum value to a field name.
const char * field_name[] =
{
	"active",
	"scope",
	"client_id",
	"sub",
	"username",
	"aud",
	"iss",
	"exp",
	"iat",
	"nbf",
	"email",
};

json_type field_type[] =
{
	json_type_boolean, // ACTIVE
	json_type_string,  // SCOPE
	json_type_string,  // CLIENT_ID
	json_type_string,  // SUB
	json_type_string,  // USERNAME
	json_type_array,   // AUD
	json_type_string,  // ISS
	json_type_int,     // EXP
	json_type_int,     // IAT
	json_type_int,     // NBF
	json_type_string,  // EMAIL
};

/*
 * We convert a JSON-like string to a JSON obj. The string is JSON-compliant
 * except that we allow single quotes in place of double quotes in order to
 * make the call site cleaner.
 */
jobj_t *
create_json_object(const char * json_string)
{
	// Copy out the string so we can modify it
	char * j_string_copy = strdup(json_string);

	// Replace all single quotes with double quotes
	char * single_quote;
	while ((single_quote = strchr(j_string_copy, '\'')) != NULL)
	{
		*single_quote = '"';
	}

	// Create the JSON object
	jobj_t * jobj = jobj_init(j_string_copy, NULL);

	// Free the temporary string
	free (j_string_copy);

	return jobj;
}

/*
 * json_string is an introspect result in a format acceptable to create_json_object().
 */
struct introspect *
create_introspect_doc(const char * json_string)
{
	jobj_t * jobj = create_json_object(json_string);
	struct introspect * i = introspect_init(jobj);
	jobj_fini(jobj);
	return i;
}

char *
create_json_string(const char * string_array[])
{
	//
	// Determine the length of the final string.
	//
	int len = 3; // "{}\0"

	for (int i = 0; string_array[i]; i++)
	{
		len += strlen(string_array[i]);
	}

	// Allocate the string.
	char * json_string = calloc(1, len);

	//
	// Build the JSON envelope.
	//
	char * ptr = json_string;
	*ptr++ = '{';
	for (int i = 0; string_array[i]; i++)
	{
		strcpy(ptr, string_array[i]);
		ptr += strlen(string_array[i]);
	}
	*ptr++ = '}';

	return json_string;
}

/*******************************************
 *              MOCKS
 *******************************************/

/*
 * Mock the logger() to prevent test errors from ending up in syslog. And
 * it allows us to make sure the error condition we caused is the error
 * condition we expected.
 */
void
logger(log_type_t type, const char * format, ...)
{
	char log_message[128];

	va_list ap;
	va_start(ap, format);
	{
		vsnprintf(log_message, sizeof(log_message), format, ap);
	}
	va_end(ap);
	check_expected(log_message);
}

/*******************************************
 *              TESTS
 *******************************************/
void
test_inactive_token(void ** state)
{
	struct introspect * i = create_introspect_doc("{ 'active': false }");

	assert_non_null(i);
	assert_false(i->active);
	assert_null(i->scope);
	assert_null(i->client_id);
	assert_null(i->sub);
	assert_null(i->username);
	assert_null(i->aud);
	assert_null(i->iss);
	assert_null(i->exp);
	assert_null(i->iat);
	assert_null(i->nbf);
	assert_null(i->email);
	assert_null(i->identities_set);
	assert_null(i->session_info);

	introspect_fini(i);
}

void
test_minimum_token(void ** state)
{
	char * json_string = create_json_string(minimum_required_fields);
	struct introspect * i = create_introspect_doc(json_string);

	assert_non_null(i);
	assert_true(i->active);
	assert_string_equal(i->scope, "scopes");
	assert_string_equal(i->client_id, "client_id");
	assert_string_equal(i->sub, "sub");
	assert_string_equal(i->username, "username");
	assert_string_equal(i->aud[0], "aud");
	assert_string_equal(i->iss, "iss");
	assert_int_equal(i->exp, 1);
	assert_int_equal(i->iat, 2);
	assert_int_equal(i->nbf, 3);
	assert_string_equal(i->email, "email");
	assert_null(i->identities_set);
	assert_null(i->session_info);

	introspect_fini(i);
	free(json_string);
}

void
test_missing_required_field(void ** state)
{
	for (int field = ACTIVE; minimum_required_fields[field] != NULL; field++)
	{
		//
		// We expect the error to generate a log message. If we verify the log message,
		// we can be certain we caused the error we expected.
		//
		char log_message[128];
		snprintf(log_message, sizeof(log_message), "Introspect record is missing required key '%s'", field_name[field]);
		expect_string(logger, log_message, log_message);

		// Save the good field value
		const char * save_field = minimum_required_fields[field];
		// Remove the required field
		minimum_required_fields[field] = "";
		// Creat the JSON string
		char * json_string = create_json_string(minimum_required_fields);
		// Create the introspect struct
		struct introspect * i = create_introspect_doc(json_string);
		// Check that introspect parsing failed
		assert_null(i);
		// Restore the good field value
		minimum_required_fields[field] = save_field;
		// Free the JSON string
		free(json_string);
	}
}

void
test_wrong_field_type(void ** state)
{
	// The type_value_map gives us string versions of valid values for each
	// JSON type.
	struct type_value_map {
		json_type    type;
		const char * value;
	} type_value_map[] = {
		{json_type_int, "1"},
		{json_type_string, "'string'"},
		{json_type_boolean, "true"},
		{json_type_array, "['str1', 'str2']"},
		{json_type_null, "null"},
	};

	// For each field...
	for (int field = ACTIVE; minimum_required_fields[field] != NULL; field++)
	{
		// For each type...
		for (int t = 0; t < sizeof(type_value_map)/sizeof(struct type_value_map); t++)
		{
			// Skip the valid value type for this field
			if (field_type[field] == type_value_map[t].type)
				continue;

			//
			// We expect the error to generate a log message. If we verify the log message,
			// we can be certain we caused the error we expected.
			//
			char log_message[128];
			snprintf(log_message, sizeof(log_message), "Introspect record has wrong type for required key '%s'", field_name[field]);
			expect_string(logger, log_message, log_message);

			// Save the good field value
			const char * save_field = minimum_required_fields[field];

			// Reconstruct the field but with the wrong type
			char new_field[128];
			snprintf(new_field, sizeof(new_field), "'%s': %s,", field_name[field], type_value_map[t].value);
			minimum_required_fields[field] = new_field;

			// Creat the JSON string
			char * json_string = create_json_string(minimum_required_fields);
			// Create the introspect struct
			struct introspect * i = create_introspect_doc(json_string);
			// Check that introspect parsing failed
			assert_null(i);
			// Restore the good field value
			minimum_required_fields[field] = save_field;
			// Free the JSON string
			free(json_string);
		}
	}
}

void
test_identities_set(void ** state)
{
	//
	// Test '"identities_set": []'
	//
	minimum_required_fields[OTHER] = "'identities_set': []";
	// Creat the JSON string
	char * json_string = create_json_string(minimum_required_fields);
	// Create the introspect struct
	struct introspect * i = create_introspect_doc(json_string);
	// Check that introspect parsing succeeded
	assert_non_null(i);
	// List should be empty
	assert_null(i->identities_set[0]);
	// Restore the good field value
	minimum_required_fields[OTHER] = NULL;
	// Free the introspect record
	introspect_fini(i);
	// Free the JSON string
	free(json_string);

	//
	// Test '"identities_set": null'
	//
	minimum_required_fields[OTHER] = "'identities_set': null";
	// Creat the JSON string
	json_string = create_json_string(minimum_required_fields);
	// Create the introspect struct
	i = create_introspect_doc(json_string);
	// Check that introspect parsing succeeded
	assert_non_null(i);
	// List should be null
	assert_null(i->identities_set);
	// Restore the good field value
	minimum_required_fields[OTHER] = NULL;
	// Free the introspect record
	introspect_fini(i);
	// Free the JSON string
	free(json_string);

	//
	// Test '"identities_set": ["id1"]'
	//
	minimum_required_fields[OTHER] = "'identities_set': ['id1']";
	// Creat the JSON string
	json_string = create_json_string(minimum_required_fields);
	// Create the introspect struct
	i = create_introspect_doc(json_string);
	// Check that introspect parsing succeeded
	assert_non_null(i);
	// Check first entry
	assert_string_equal(i->identities_set[0], "id1");
	// List should have length 1
	assert_null(i->identities_set[1]);
	// Restore the good field value
	minimum_required_fields[OTHER] = NULL;
	// Free the introspect record
	introspect_fini(i);
	// Free the JSON string
	free(json_string);

	//
	// Test '"identities_set": ["id1", "id2"]'
	//
	minimum_required_fields[OTHER] = "'identities_set': ['id1', 'id2']";
	// Creat the JSON string
	json_string = create_json_string(minimum_required_fields);
	// Create the introspect struct
	i = create_introspect_doc(json_string);
	// Check that introspect parsing succeeded
	assert_non_null(i);
	// Check entries
	assert_string_equal(i->identities_set[0], "id1");
	assert_string_equal(i->identities_set[1], "id2");
	// List should have length 2
	assert_null(i->identities_set[2]);
	// Restore the good field value
	minimum_required_fields[OTHER] = NULL;
	// Free the introspect record
	introspect_fini(i);
	// Free the JSON string
	free(json_string);
}

void
test_session_info(void ** state)
{
	//
	// Test '"session_info": null
	//
	minimum_required_fields[OTHER] = "'session_info': null";
	// Creat the JSON string
	char * json_string = create_json_string(minimum_required_fields);
	// Create the introspect struct
	struct introspect * i = create_introspect_doc(json_string);
	// Check that introspect parsing succeeded
	assert_non_null(i);
	// Session should be null since it is optional
	assert_null(i->session_info);
	// Restore the good field value
	minimum_required_fields[OTHER] = NULL;
	// Free the introspect record
	introspect_fini(i);
	// Free the JSON string
	free(json_string);

	//
	// Test '"session_info": {'session_id': 'sid1'}'
	//
	minimum_required_fields[OTHER] = "'session_info': {'session_id': 'sid1'}";
	// Creat the JSON string
	json_string = create_json_string(minimum_required_fields);
	// Create the introspect struct
	i = create_introspect_doc(json_string);
	// Check that introspect parsing succeeded
	assert_non_null(i);
	// Session ID should be there
	assert_string_equal(i->session_info->session_id, "sid1");
	// Authentications is optional so it should be NULL
	assert_null(i->session_info->authentications);
	// Restore the good field value
	minimum_required_fields[OTHER] = NULL;
	// Free the introspect record
	introspect_fini(i);
	// Free the JSON string
	free(json_string);
}

void
test_authentications(void ** state)
{
	//
	// Test '"authentications": null'
	//
	minimum_required_fields[OTHER] =
		"'session_info': {        "
		"  'session_id': 'sid1',  "
		"  'authentications': null"
		"}                        ";

	// Creat the JSON string
	char * json_string = create_json_string(minimum_required_fields);
	// Create the introspect struct
	struct introspect * i = create_introspect_doc(json_string);
	// Check that introspect parsing succeeded
	assert_non_null(i);
	// Authentications should be null because it is optional
	assert_null(i->session_info->authentications);
	// Restore the good field value
	minimum_required_fields[OTHER] = NULL;
	// Free the introspect record
	introspect_fini(i);
	// Free the JSON string
	free(json_string);

	//
	// Test '"authentications": {}'
	//
	minimum_required_fields[OTHER] =
		"'session_info': {      "
		"  'session_id': 'sid1',"
		"  'authentications': {}"
		"}                      ";

	// Creat the JSON string
	json_string = create_json_string(minimum_required_fields);
	// Create the introspect struct
	i = create_introspect_doc(json_string);
	// Check that introspect parsing succeeded
	assert_non_null(i);
	// Authentications should be zero length
	assert_null(i->session_info->authentications[0]);
	// Restore the good field value
	minimum_required_fields[OTHER] = NULL;
	// Free the introspect record
	introspect_fini(i);
	// Free the JSON string
	free(json_string);

	//
	// Test one authentication with MFA
	//
	minimum_required_fields[OTHER] =
		"'session_info': {       "
		"  'session_id': 'sid1', "
		"  'authentications': {  "
		"    'id1': {            "
		"      'auth_time': 120, "
		"      'idp': 'idp1',    "
		"      'amr': ['mfa']    "
		"    }                   "
		"  }                     "
		"}                       ";

	// Creat the JSON string
	json_string = create_json_string(minimum_required_fields);
	// Create the introspect struct
	i = create_introspect_doc(json_string);
	// Check that introspect parsing succeeded
	assert_non_null(i);
	// Verify the authentication
	assert_string_equal(i->session_info->authentications[0]->identity_id, "id1");
	assert_int_equal(i->session_info->authentications[0]->auth_time, 120);
	assert_string_equal(i->session_info->authentications[0]->idp, "idp1");
	assert_true(i->session_info->authentications[0]->amr.mfa);
	// Authentications should be length 1
	assert_null(i->session_info->authentications[1]);
	// Restore the good field value
	minimum_required_fields[OTHER] = NULL;
	// Free the introspect record
	introspect_fini(i);
	// Free the JSON string
	free(json_string);


	//
	// Test four authentications:
	//  - one without 'amr'
	//  - one with an empty 'amr'
	//  - one with an 'amr' with multiple values, one of which is mfa
	//  - one with an 'amr' set to 'null'
	//
	minimum_required_fields[OTHER] =
		"'session_info': {           "
		"  'session_id': 'sid1',     "
		"  'authentications': {      "
		"    'id1': {                "
		"      'auth_time': 120,     "
		"      'idp': 'idp1'         "
		"    },                      "
		"    'id2': {                "
		"      'auth_time': 60,      "
		"      'idp': 'idp2',        "
		"      'amr': []             "
		"    },                      "
		"    'id3': {                "
		"      'auth_time': 30,      "
		"      'idp': 'idp3',        "
		"      'amr': ['foo', 'mfa'] "
		"    },                      "
		"    'id4': {                "
		"      'auth_time': 15,      "
		"      'idp': 'idp4',        "
		"      'amr': null           "
		"    }                       "
		"  }                         "
		"}                           ";

	// Creat the JSON string
	json_string = create_json_string(minimum_required_fields);
	// Create the introspect struct
	i = create_introspect_doc(json_string);
	// Check that introspect parsing succeeded
	assert_non_null(i);
	// Verify authentication 1
	assert_string_equal(i->session_info->authentications[0]->identity_id, "id1");
	assert_int_equal(i->session_info->authentications[0]->auth_time, 120);
	assert_string_equal(i->session_info->authentications[0]->idp, "idp1");
	assert_false(i->session_info->authentications[0]->amr.mfa);
	// Verify authentication 2
	assert_string_equal(i->session_info->authentications[1]->identity_id, "id2");
	assert_int_equal(i->session_info->authentications[1]->auth_time, 60);
	assert_string_equal(i->session_info->authentications[1]->idp, "idp2");
	assert_false(i->session_info->authentications[1]->amr.mfa);
	// Verify authentication 3
	assert_string_equal(i->session_info->authentications[2]->identity_id, "id3");
	assert_int_equal(i->session_info->authentications[2]->auth_time, 30);
	assert_string_equal(i->session_info->authentications[2]->idp, "idp3");
	assert_true(i->session_info->authentications[2]->amr.mfa);
	// Verify authentication 4
	assert_string_equal(i->session_info->authentications[3]->identity_id, "id4");
	assert_int_equal(i->session_info->authentications[3]->auth_time, 15);
	assert_string_equal(i->session_info->authentications[3]->idp, "idp4");
	assert_false(i->session_info->authentications[3]->amr.mfa);
	// Authentications should be length 4
	assert_null(i->session_info->authentications[4]);
	// Restore the good field value
	minimum_required_fields[OTHER] = NULL;
	// Free the introspect record
	introspect_fini(i);
	// Free the JSON string
	free(json_string);
}

/*******************************************
 *              FIXTURES
 *******************************************/

int
main()
{
	const struct CMUnitTest tests[] = {
		{"inactive token", test_inactive_token},
		{"minimum token", test_minimum_token},
		{"missing required field", test_missing_required_field},
		{"wrong field type", test_wrong_field_type},
		{"identities_set", test_identities_set},
		{"session_info", test_session_info},
		{"authentications", test_authentications},
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
