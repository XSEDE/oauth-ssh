#ifndef _GLOBUS_AUTH_RESPONSE_H_
#define _GLOBUS_AUTH_RESPONSE_H_

struct _response {
	int  length;
	char * body;
};

void
_response_free(struct _response);

#endif /* _GLOBUS_AUTH_RESPONSE_H_ */
