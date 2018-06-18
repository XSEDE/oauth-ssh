
struct fopen_mock {
	int    errnum; /* errno */
	FILE * fptr;
};

FILE *
__wrap_fopen(const char *path, const char *mode)
{
	FILE * __real_fopen(const char *path, const char *mode);

	check_expected(path);

	struct fopen_mock * m = (struct fopen_mock *) mock();

	if (!m)
		return __real_fopen(path, mode);

	errno = m->errnum;
	return m->fptr;
}

struct fclose_mock {
	int errnum; /* errno */
	int num;
};

int
__wrap_fclose(FILE *fp)
{
	int __real_fclose(FILE *fp);

	check_expected(fp);

	struct fclose_mock * m = (struct fclose_mock *) mock();

	if (!m)
		return __real_fclose(fp);

	errno = m->errnum;
	return m->num;
}

struct fgets_mock {
	int    errnum; /* errno */
	char * cptr;
};

char *
__wrap_fgets(char *s, int size, FILE *stream)
{
	char * __real_fgets(char *s, int size, FILE *stream);

	struct fgets_mock * m = (struct fgets_mock *) mock();

	if (!m)
		return __real_fgets(s, size, stream);

	errno = m->errnum;

	if (m->cptr)
	{
		strcpy(s, m->cptr);
		return s;
	}

	return NULL;
}

