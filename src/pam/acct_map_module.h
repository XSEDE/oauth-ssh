#ifndef _ACCT_MAP_MODULE_H_
#define _ACCT_MAP_MODULE_H_

struct acct_map_module {
	int    (*initialize)(void ** handle, const char * option);
	char * (*lookup)    (void *  handle, const char * username_or_uuid);
	void   (*finalize)  (void *  handle);
};

#endif /* _ACCT_MAP_MODULE_H_ */
