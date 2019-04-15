/*
 * System includes.
 */
#include <pthread.h>
#include <syslog.h>
#include <string.h>

/*
 * Local includes.
 */
#include "logger.h"
#include "debug.h" // always last

static log_type_t minimum_log_level = LOG_TYPE_INFO;

void
logger_init(int flags, int argc, const char ** argv)
{
	for (int i = 0; i < argc; i++)
	{
		if (strcmp(argv[i], "debug") == 0)
			minimum_log_level = LOG_TYPE_DEBUG;
	}
}

static void
initialize()
{
	openlog("Globus SSH", 0, LOG_AUTH);
}

void
logger(log_type_t type, const char * format, ...)
{
	static pthread_once_t initialized = PTHREAD_ONCE_INIT;
	pthread_once(&initialized, initialize);

	if (type < minimum_log_level)
		return;

	int priority = 0;

	switch (type)
	{
	case LOG_TYPE_INFO:
		priority = LOG_INFO;
		break;
	case LOG_TYPE_ERROR:
		priority = LOG_ERR;
		break;
	case LOG_TYPE_DEBUG:
		priority = LOG_DEBUG;
		break;
#ifdef DEBUG
	default:
		ASSERT(0);
#endif
	}

	va_list ap;

	va_start(ap, format);
	vsyslog(priority, format, ap);
	va_end(ap);
}
