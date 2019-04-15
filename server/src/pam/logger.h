#ifndef _LOGGER_H_
#define _LOGGER_H_

/*
 * System includes.
 */
#include <stdarg.h>

typedef enum {
	LOG_TYPE_DEBUG,
	LOG_TYPE_INFO,
	LOG_TYPE_ERROR,
} log_type_t;

void logger_init(int flags, int argc, const char ** argv);
void logger(log_type_t, const char * format, ...);

#endif /* _LOGGER_H_ */
