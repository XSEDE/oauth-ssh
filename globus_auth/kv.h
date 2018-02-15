#ifndef _GLOBUS_AUTH_KV_H_
#define _GLOBUS_AUTH_KV_H_

struct _kvs {
        struct _kv * kv;
        int          cnt;
};

// Skips kv's with value == NULL
struct _kvs
_kvs_copy(struct _kvs kvs);

void
_kvs_free(struct _kvs kvs);

// XXX circular dependency
#include "strings.h"

#define _P_KV(k, v) (struct _kv) {k, v, 0} // Public info
#define _S_KV(k, v) (struct _kv) {k, v, 1} // Secret info

struct _kv {
        struct _string key;
        struct _string val;
        short sanitize; // !0 if should not show in debug
};

void
_kv_free(struct _kv kv);

#endif /* _GLOBUS_AUTH_KV_H_ */
