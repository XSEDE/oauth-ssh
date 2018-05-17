/*
 * Copyright The University of Chicago
 * All Rights Reserved.
 */
#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#define PAM_SM_AUTH
#include <security/pam_modules.h>

int
get_client_access_token(struct pam_conv *  conv,
                        const char      *  prompt,
                        char            ** access_token);

int
display_client_message(struct pam_conv * conv, 
                       const char      * message);

char * 
join_string_list(const char * const strings[],
                 const char delimiter);

char * 
join_strings(const char * string1, 
             const char * string2, 
             const char   delimiter);

char *
build_acct_map_msg(const char * const accounts[]);

int
string_in_list(const char * string, 
               const char * const list[]);

#endif /* _UTILITIES_H_ */
