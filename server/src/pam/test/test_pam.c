/*
 * System includes.
 */
#include <string.h>
#include <stdlib.h>

#define PAM_SM_AUTH
#include <security/pam_modules.h>

/*
 * Local includes.
 */
#include "logger.h"
#include "config.h"
#include "reply.h"
#include "debug.h" // always last

typedef int pam_status_t;

/*******************************************
 *              MOCKS
 *******************************************/
void
logger_init(int flags, int argc, const char ** argv)
{
	check_expected(flags);
	check_expected(argc);
	check_expected(argv);
}

void logger(log_type_t type, const char * format, ...) {}
struct config * config_init(int flags, int argc, const char ** argv)
{return NULL;}
int pam_get_item(const pam_handle_t *pamh, int item_type,
                        const void **item) {return 0;}

//      struct pam_message {
//           int msg_style;
//           const char *msg;
//       };

//       struct pam_response {
//           char *resp;
//           int resp_retcode;
//       };

//       struct pam_conv {
//           int (*conv)(int num_msg, const struct pam_message **msg,
//                       struct pam_response **resp, void *appdata_ptr);
//           void *appdata_ptr;
//       };

void config_fini(struct config * c) {}
void reply_fini(struct reply * r) {}

pam_status_t
process_command(pam_handle_t  *  pamh,
                struct config *  config,
                const char    *  user_input,
                struct reply  ** reply) {return 0;}



/*******************************************
 *              TESTS
 *******************************************/
void
test_(void ** state)
{
	int flags = 0;
	int argc = 0;
	const char * argv[argc];
	expect_value(logger_init, flags, flags);
	expect_value(logger_init, argc, argc);
	expect_value(logger_init, argv, argv);
}

/*******************************************
 *              FIXTURES
 *******************************************/

int
main()
{
	const struct CMUnitTest tests[] = {
		{"base64", test_},
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
