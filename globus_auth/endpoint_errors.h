#ifndef _GLOBUS_AUTH_ENDPOINT_ERRORS_H_
#define _GLOBUS_AUTH_ENDPOINT_ERRORS_H_

// Single API Error
struct _ep_error {
	int    status;
	char * id;
	char * code;
	char * detail;
	char * title;
};

struct _ep_errors {
	int                error_cnt;
	struct _ep_error * errors;
	char             * error_description;
	char             * error;
};

// NULL if this is not an error message or j_string is NULL
struct _ep_errors *
_ep_errors_from_json_string(const char * j_string);

void
_ep_errors_free(struct _ep_errors *);

#endif /* _GLOBUS_AUTH_ENDPOINT_ERRORS_H_ */
