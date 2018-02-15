#include <string.h>
#include <stdio.h>
#include <globus_auth.h>

int
main(int argc, const char * argv[])
{
	const char * bearer_token = argv[1];

	if (argc !=2)
	{
		fprintf(stderr, "Usage: %s <bearer_token>\n", argv[0]);
		return 1;
	}

	struct auth_userinfo * userinfo = NULL;

	int retval = auth_get_userinfo(bearer_token, &userinfo);

	printf("sub: %s\n", userinfo->sub ? userinfo->sub : "null");
	printf("preferred_username: %s\n", userinfo->preferred_username ? userinfo->preferred_username : "null");
	printf("name: %s\n", userinfo->name ? userinfo->name : "null");
	printf("email: %s\n", userinfo->email ? userinfo->email : "null");
	
	auth_free_userinfo(userinfo);
	return retval;
}

