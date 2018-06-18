
struct http_mock {
	int    return_value;
	char * reply_body;
	char * error_msg;
};

int
http_get_request(struct credentials *  credentials,
                 const char         *  request_url,
                 char               ** reply_body,
                 char               ** error_msg)
{
	struct http_mock * m = (struct http_mock *) mock();

	*reply_body = m->reply_body;
	*error_msg  = m->error_msg;
	return m->return_value;
}

int
http_post_request(struct credentials *  credentials,
                  const char         *  request_url,
                  const char         *  request_body,
                  char               ** reply_body,
                  char               ** error_msg)
{
	struct http_mock * m = (struct http_mock *) mock();

	*reply_body = m->reply_body;
	*error_msg  = m->error_msg;
	return m->return_value;
}
