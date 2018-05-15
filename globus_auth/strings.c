#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>

#include "strings.h"
#include "curl_wrapper.h"
#include "test/unit_test.h"

void
_string_free(struct _string s)
{
	//ASSERT(s.str);
	if (s.own_it)
		free(s.str);
}

char *
_strings_build(const char * format, ...)
{
	int length = 0;
	char * string = NULL;

	va_list ap1, ap2;

	va_start(ap1, format); va_copy(ap2, ap1);
	{
		length = vsnprintf(NULL, 0, format, ap1);
		string = calloc(length + 1, sizeof(char));
		vsnprintf(string, length+1, format, ap2);
	}
	va_end(ap1); va_end(ap2);
	return string;
}

char *
_strings_build_list(char delimiter, const char ** value)
{
	int count  = 0;
	int length = 0;

	while (value[count]) length += strlen(value[count++]);
	char * list = calloc(length + count + 1, 1);

	count = 0;
	while (value[count])
	{
		strcat(list, value[count]);
		if (value[++count])
			list[strlen(list)] = delimiter;
	}

	return list;
}

char *
_strings_build_kv_list(struct _kvs kvs,
                       const char  delimiter,
                       int         url_encode,
                       int         sanitize)
{
	if (kvs.cnt == 0) return NULL;

	char * values[kvs.cnt];
	for (int i = 0; i < kvs.cnt; i++)
	{
		values[i] = kvs.kv[i].val.str;
		if (url_encode)
			values[i] = _curl_safe_escape(values[i], strlen(values[i]));
	}

	int length = 0;
	for (int i = 0; i < kvs.cnt; i++)
	{
		length += strlen(values[i]);
		length += strlen(kvs.kv[i].val.str);
	}
	length += 2*kvs.cnt;

	char * string = calloc(sizeof(char), length);

	for (int i = 0; i < kvs.cnt; i++)
	{
		if (i) string[strlen(string)] = delimiter;
		strcat(string, kvs.kv[i].key.str);
		string[strlen(string)] = '=';
		strcat(string, values[i]);
		if (url_encode) free(values[i]);

		if (sanitize && kvs.kv[i].sanitize)
		{
			for (int j = 0; j < strlen(values[i]); j++)
			{
				string[strlen(string) - 1 - j] = 'X';
			}
		}
	}

	return string;
}

char *
_strings_build_url(const char * fqdn_n_path, struct _kvs query_kvs, int sanitize)
{
	char * kv_list = _strings_build_kv_list(query_kvs, '&', 1, sanitize);
	char * url     = _strings_build("%s%s%s", 
	                                fqdn_n_path, 
	                                kv_list ? "?" : "", 
	                                kv_list ? kv_list: "");
	if (kv_list) free(kv_list);
	return url;
}

char **
_strings_split_to_array(const char * string)
{
	int cnt = 0;
	const char * first = string;
	const char * last  = string;

	char ** array = NULL;

	if (!string) return NULL;
	while (*first != '\0')
	{
		while (!isspace(*last) && *last != '\0') last++;
		cnt++;
		while (isspace(*last) && *last != '\0') last++;
		first = last;
	}

	if (!cnt) return NULL;

	array = calloc(sizeof(char *), cnt + 1);

	first = last = string;
	cnt   = 0;
	while (*first != '\0')
	{
		while (!isspace(*last) && *last != '\0') last++;
		array[cnt++] = strndup(first, last - first);
		while (isspace(*last) && *last != '\0') last++;
		first = last;
	}

	return array;
}

