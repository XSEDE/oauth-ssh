#ifndef _PARSER_H_
#define _PARSER_H_

/*
 * System includes.
 */
#include <stdbool.h>
#include <stdio.h>

bool
read_next_pair(FILE * fptr, char ** key, char *** values);

#endif /* _PARSER_H_ */
