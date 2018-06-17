
static int
initialize(void ** handle, const char * option)
{
    *handle = (void *)0xDEADBEEF; // Fool acct_map to think we are active
    return mock();
}

static char *
lookup(void * handle, const char * id)
{
	check_expected(id);

    char * value = (char *)mock();
	if (value)
		value = strdup(value);
    return value;
}

static void
finalize(void * handle)
{
}

struct acct_map_module acct_map_module_mock = {
    initialize, lookup, finalize
};
