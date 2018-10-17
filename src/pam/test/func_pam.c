
/*
 * System includes.
 */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#define PAM_SM_AUTH
#include <security/pam_modules.h>
#include <security/pam_appl.h>

extern int
pam_sm_authenticate(pam_handle_t *  pamh,
                    int             flags,
                    int             argc,
                    const char   ** argv);


int
pam_conv(int                         num_msg, 
         const struct pam_message ** msg,
         struct pam_response      ** resp, 
         void                     *  appdata_ptr)
{
	*resp = calloc(num_msg, sizeof(struct pam_response));

	for (int i = 0; i < num_msg; i++)
	{
		switch (msg[i]->msg_style)
		{
		case PAM_PROMPT_ECHO_OFF:
		case PAM_PROMPT_ECHO_ON:
		{
			if (appdata_ptr)
			{
				(*resp)[i].resp = strdup((char *)appdata_ptr);
				break;
			}

			printf("%s", msg[i]->msg);
			char buf[128];
			if (fgets(buf, sizeof(buf), stdin))
			{
				if (buf[strlen(buf)-1] == '\n')
					buf[strlen(buf)-1] = '\0';
				(*resp)[i].resp = strdup(buf);
			}
			break;
		}
		case PAM_ERROR_MSG:
		case PAM_TEXT_INFO:
			printf("%s\n", msg[i]->msg);
			break;
		}
	}
	return PAM_SUCCESS;
}

void
print_pam_error(int retval)
{
	switch (retval)
	{
	case PAM_AUTH_ERR:
		printf("PAM_AUTH_ERR\n");
		break;
	case PAM_CRED_INSUFFICIENT:
		printf("PAM_CRED_INSUFFICIENT\n");
		break;
	case PAM_AUTHINFO_UNAVAIL:
		printf("PAM_AUTHINFO_UNAVAIL\n");
		break;
	case PAM_SUCCESS:
		printf("PAM_SUCCESS\n");
		break;
	case PAM_USER_UNKNOWN:
		printf("PAM_USER_UNKNOWN\n");
		break;
	case PAM_MAXTRIES:
		printf("PAM_MAXTRIES\n");
		break;
	default:
		printf("UNKNOWN RETURN VALUE %d\n", retval);
		break;
	}
}

void
usage(const char * program)
{
	fprintf(stderr, "Usage: %s [options] [config_file]\n", program);
	fprintf(stderr, "\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "\t-a <account> (required)\n");
	fprintf(stderr, "\t-t <token>   (optional)\n");
}

int
main(int argc, const char * argv[])
{
	char * token   = NULL;
	char * account = NULL;

	int opt;
	while ((opt = getopt(argc, (char * const *)argv, "a:t:")) != -1)
	{
		switch (opt)
		{
		case 't':
			token = optarg;
			break;
		case 'a':
			account = optarg;
			break;
		default:
			usage(argv[0]);
			return 1;
		}
	}
	if (!account)
	{
		usage(argv[0]);
		return 1;
	}

	struct pam_conv pam_conversation = {pam_conv, token};

	pam_handle_t * pamh = NULL;
	int retval = pam_start(argv[0], account, &pam_conversation, &pamh);
	if (retval != PAM_SUCCESS)
	{
		printf("pam_start failed with ");
		print_pam_error(retval);
		return retval;
	}

	retval = pam_sm_authenticate(pamh, 0, argc-optind, &argv[optind]);
	print_pam_error(retval);

	pam_end(pamh, retval);

	return retval;
}
