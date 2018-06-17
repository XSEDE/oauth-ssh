/*
 * System includes.
 */
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

/*
 * Local includes.
 */
#include "auth.h"
#include "identities.h"
#include "http.h"
#include "json.h"
#include "strings.h"

const char * identities_endpoint = GLOBUS_AUTH_URL "/v2/api/identities";

static int
perform_request(struct credentials *  credentials,
                const char         *  request_url,
                json_t             ** json_reply_body,
                char               ** error_msg);

static void
parse_json_replies(json_t          *   json_uuid, 
                   json_t          *   json_user,
                   struct identity *** identities,
                   char            **  error_msg);

static struct identity *
json_to_identity(json_t * json);

static void
find_unique_ids(json_t * json1,
                json_t * json2,
                void (*callback)(json_t * jobj_id, void * callback_arg),
                void   * callback_arg);

static char * 
build_uuid_request(const char * ids[]);

static char * 
build_username_request(const char ** ids);

static int
is_uuid(const char * s);

static int
is_username(const char * s);

int
get_identities(struct credentials *   credentials,
                    const char         **  ids,
                    struct identity    *** identities,
                    char               **  error_msg)
{
	int retval = 0;
	json_t * json_uuid = NULL;
	json_t * json_user = NULL;

	char * request_url = build_uuid_request(ids);
	if (request_url)
	{
		retval = perform_request(credentials,
		                         request_url,
		                         &json_uuid, 
		                         error_msg);
		free(request_url);
		if (retval) goto cleanup;
	}

	request_url = build_username_request(ids);
	if (request_url)
	{
		retval = perform_request(credentials,
		                         request_url,
		                         &json_user, 
		                         error_msg);
		free(request_url);
		if (retval) goto cleanup;
	}

	parse_json_replies(json_uuid, json_user, identities, error_msg);

cleanup:
	if (json_user)
		json_free(json_user);
	if (json_uuid)
		json_free(json_uuid);

	return retval;
}

void
free_identity(struct identity * id)
{
	if (id->id) free(id->id);
	if (id->username) free(id->username);
	if (id->identity_provider) free(id->identity_provider);
	if (id->email) free(id->email);
	if (id->name) free(id->name);
	if (id->organization) free(id->organization);
	free(id);
}

void
free_identities(struct identity * ids[])
{
	for (int i = 0; ids[i]; i++)
	{
		free_identity(ids[i]);
	}
	free(ids);
}

static int
perform_request(struct credentials *  credentials,
                const char         *  request_url,
                json_t             ** json_reply_body,
                char               ** error_msg)
{
	*json_reply_body = NULL;

	const char * list[] = {identities_endpoint, request_url, NULL};

	char * url = join(list, '?');
	char * reply_body = NULL;
	int retval = http_get_request(credentials,
	                              url,
	                              &reply_body,
	                              error_msg);
	free(url);

	if (retval || !reply_body)
		goto cleanup;

	*json_reply_body = json_init(reply_body, error_msg);

cleanup:
	if (reply_body)
		free(reply_body);
	return *json_reply_body == NULL ? 1 : retval;
}

struct add_identity {
	int count;
	struct identity ** identities;
};

void
add_identity(json_t * jobj, void * cb_arg)
{
	struct add_identity * arg = cb_arg;

	arg->identities = realloc(arg->identities, (arg->count+2) * sizeof(void *));
	arg->identities[arg->count++] = json_to_identity(jobj);
	arg->identities[arg->count] = NULL;
}

static void
parse_json_replies(json_t          *   json_uuid, 
                   json_t          *   json_user,
                   struct identity *** identities,
                   char            **  error_msg)
{
	struct add_identity arg = {0, NULL};
	find_unique_ids(json_uuid, json_user, add_identity, &arg);
	*identities = arg.identities;
}

static struct identity *
json_to_identity(json_t * json)
{
	struct identity * i = calloc(sizeof(struct identity), 1);

	i->id           = safe_strdup(json_get_string(json, "id"));
	i->username     = safe_strdup(json_get_string(json, "username"));
	i->email        = safe_strdup(json_get_string(json, "email"));
	i->name         = safe_strdup(json_get_string(json, "name"));
	i->organization = safe_strdup(json_get_string(json, "organization"));
	i->identity_provider = safe_strdup(json_get_string(json, "identity_provider"));

	const char * status  = json_get_string(json, "status");

	if (strcmp(status, "unused") == 0)
		i->status = ID_STATUS_UNUSED;
	else if (strcmp(status, "used") == 0)
		i->status = ID_STATUS_USED;
	else if (strcmp(status, "private") == 0)
		i->status = ID_STATUS_PRIVATE;
	else if (strcmp(status, "closed") == 0)
		i->status = ID_STATUS_CLOSED;
#ifdef DEBUG
	else ASSERT(0);
#endif

	return i;
}

static int
uuid_unique(json_t * j_arr, int max_index, const char * uuid)
{
	if (!j_arr)
		return 1;

	if (max_index == -1)
		max_index = json_array_len(j_arr);

	for (int i = 0; i < max_index; i++)
	{
		json_t * j_id = json_array_idx(j_arr, i);

		if (strcmp(json_get_string(j_id, "id"), uuid) == 0)
		{
			json_free(j_id);
			return 0;
		}
		json_free(j_id);
	}

	return 1;
}

static void
find_unique_ids(json_t * json1,
                json_t * json2,
                void (*callback)(json_t * jobj_id, void * callback_arg),
                void   * callback_arg)
{
	json_t * j_arr1 = NULL;
	if (json1)
	{
		j_arr1 = json_get_array(json1, "identities");
		for (int i = 0; i < json_array_len(j_arr1); i++)
		{
			json_t * j_id = json_array_idx(j_arr1, i);
			const char * uuid  = json_get_string(j_id, "id");

			if (uuid_unique(j_arr1, i, uuid))
				callback(j_id, callback_arg);

			json_free(j_id);
		}
	}

	json_t * j_arr2 = NULL;
	if (json2)
	{
		j_arr2 = json_get_array(json2, "identities");
		for (int i = 0; i < json_array_len(j_arr2); i++)
		{
			json_t * j_id = json_array_idx(j_arr2, i);
			const char * uuid  = json_get_string(j_id, "id");

			if (uuid_unique(j_arr1, -1, uuid) && uuid_unique(j_arr2, i, uuid))
				callback(j_id, callback_arg);

			json_free(j_id);
		}
	}

	if (j_arr1)
		json_free(j_arr1);
	if (j_arr2)
		json_free(j_arr2);
}

static char * 
build_uuid_request(const char * ids[])
{
	char * request = NULL;

	for (int i = 0; ids && ids[i]; i++)
	{
		if (is_uuid(ids[i]))
		{
			char * s1 = "ids";
			char delim = '=';

			if (request)
			{
				s1 = request;
				delim = ',';
			}

			char * s = join((const char *[]) {s1, ids[i], NULL}, delim);

			if (request)
				free(request);
			request = s;
		}
	}
	return request;
}

static char * 
build_username_request(const char ** ids)
{
	char * request = NULL;

	for (int i = 0; ids && ids[i]; i++)
	{
		if (is_username(ids[i]))
		{
			char * s1 = "usernames";
			char delim = '=';

			if (request)
			{
				s1 = request;
				delim = ',';
			}

			char * s = join((const char *[]) {s1, ids[i], NULL}, delim);

			if (request)
				free(request);
			request = s;
		}
	}
	return request;
}

static int
is_uuid(const char * s)
{
	if (strlen(s) != 36)
		return 0;

	const char * uuid_regex = "[0-9a-f]{8}-[0-9a-f]{4}-[1-5][0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}";

	regex_t req;
	regcomp(&req, uuid_regex, REG_EXTENDED|REG_ICASE|REG_NOSUB);
	int retval = regexec(&req, s, 0, NULL, 0);

	regfree(&req);

	return (retval != REG_NOMATCH);
}

static int
is_username(const char * s)
{
	return (strchr(s, '@') != NULL);
}

