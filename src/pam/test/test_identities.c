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
#include "identities.h"
#include "json.h"
#include "debug.h" // always last

const char * jstring = 
"{  "
"	\"included\": {"
"		\"identity_providers\": ["
"			{"
"				\"id\": \"idp1\","
"				\"name\": \"name1\""
"			},"
"			{"
"				\"id\": \"idp2\","
"				\"name\": \"name2\""
"			}"
"		]"
"	},"
"	\"identities\": [ "
"		{"
"			\"username\": \"username1\","
"			\"status\": \"status1\","
"			\"name\": \"name1\","
"			\"id\": \"id1\","
"			\"identity_provider\": \"idp1\","
"			\"organization\": \"org1\","
"			\"email\": \"email1\","
"		},"
"		{"
"			\"username\": \"username2\","
"			\"status\": \"status2\","
"			\"name\": \"name2\","
"			\"id\": \"id2\","
"			\"identity_provider\": \"idp2\","
"			\"organization\": \"org2\","
"			\"email\": \"email2\","
"		}"
"	]"
"}  ";

/*******************************************
 *              MOCKS
 *******************************************/
void
vsyslog(int priority, const char *format, va_list ap)
{
	mock();
}

/*******************************************
 *              TESTS
 *******************************************/
void
test_valid(void ** state)
{
	jobj_t * j = jobj_init(jstring, NULL);
	struct identities * i = identities_init(j);
	assert_non_null(i);
	identities_fini(i);
	jobj_fini(j);
}


/*******************************************
 *              FIXTURES
 *******************************************/

int
main()
{
	const struct CMUnitTest tests[] = {
		{"valid", test_valid},
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}

