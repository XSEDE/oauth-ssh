/*
 * Copyright The University of Chicago
 * All Rights Reserved.
 */

/*
 * System includes.
 */
#include <syslog.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

/*
 * Local includes.
 */
#include "introspect.h"
#include "json.h"
#include "debug.h" // always last

const char * active_token_jstring = 
"{  "
"	\"active\": true,   "
"	\"scope\": \"scopes\",   "
"	\"client_id\": \"client_id\",   "
"	\"sub\": \"sub\",   "
"	\"username\": \"username\",   "
"	\"aud\": [\"aud\"],   "
"	\"iss\": \"iss\",   "
"	\"exp\": 1,   "
"	\"iat\": 2,   "
"	\"nbf\": 3,   "
"	\"email\": \"joe@example.com\",   "
"	\"identities_set\": [\"id1\", \"id2\"],    "
"	\"session_info\": {   "
"		\"session_id\": \"82bd6c39-5363-461b-a5c2-c39fc08a0148\",   "
"		\"authentications\": {   "
"			\"82bd6c39-5363-461b-a5c2-c39fc08a0148\" : {   "
"				\"idp\": \"id1\",   "
"				\"auth_time\": 30   "
"			},   "
"			\"2c3d5ff0-46dd-4bab-9d8a-b417076d8f24\" : {   "
"				\"idp\": \"id2\",   "
"				\"auth_time\": 60   "
"			}   "
"		}   "
"	}   "
"}   ";

const char * inactive_token_jstring = 
"{  "
"	\"active\": false,   "
"}   ";

/*******************************************
 *              MOCKS
 *******************************************/
void
vsyslog(int priority, const char *format, va_list ap)
{
}

/*******************************************
 *              TESTS
 *******************************************/
void
test_active_token(void ** state)
{
	jobj_t * j = jobj_init(active_token_jstring, NULL);
	struct introspect * i = introspect_init(j);
	assert_non_null(i);
	introspect_fini(i);
	jobj_fini(j);
}

void
test_inactive_token(void ** state)
{
	jobj_t * j = jobj_init(inactive_token_jstring, NULL);
	struct introspect * i = introspect_init(j);
	assert_non_null(i);
	introspect_fini(i);
	jobj_fini(j);
}

/*******************************************
 *              FIXTURES
 *******************************************/

int
main()
{
	const struct CMUnitTest tests[] = {
		{"active token", test_active_token},
		{"inactive token", test_inactive_token},
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}

