
struct config *
config_init()
{
	return NULL;
}

void
config_free(struct config * config)
{
}

int
config_load(struct config * config, const char * path)
{
	check_expected(path);
	return mock();
}

void
config_get_value(struct config *  config,
                 const char    *  key,
                 char          ** value)
{
	check_expected(key);
	*value = (char *)mock();
	if (*value)
		*value = strdup(*value);
}

