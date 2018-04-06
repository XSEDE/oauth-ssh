#include <stdlib.h>
#include "kv.h"

void
_kv_free(struct _kv kv)
{
	_string_free(kv.key);
	_string_free(kv.val);
}

// Skips kv's with value == NULL
struct _kvs
_kvs_copy(struct _kvs kvs)
{
	int cnt = 0;
	for (int index = 0; index < kvs.cnt; index++)
	{
		if (kvs.kv[index].val.str)
			cnt++;
	}

	struct _kvs new_kvs = {NULL, 0};
	if (cnt)
	{
		new_kvs.kv = calloc(sizeof(struct _kv), cnt);
		for (int index = 0; index < kvs.cnt; index++)
		{
			if (kvs.kv[index].val.str)
			{
				new_kvs.kv[new_kvs.cnt++] = kvs.kv[index];
			}
		}
		new_kvs.cnt = cnt;
	}

	return new_kvs;
}

void
_kvs_free(struct _kvs kvs)
{
	for (int i = 0; i < kvs.cnt; i++)
	{
		_kv_free(kvs.kv[i]);
	}
	if (kvs.kv)
		free(kvs.kv);
}
