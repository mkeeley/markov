#ifndef PARSE_H
#define PARSE_H

#include <assert.h>
#include <string.h>

#define MR 	"Mr."
#define MRS	"Mrs."
#define MS	"Ms."

typedef enum {
	NOTHING = 0,
	COMMA 	= 0,
	PREFIX	= 0,
	PERIOD	= 1,
	QUESTION= 1,
	BANG	= 1
} ENDING;

ENDING parse(char *word);

#endif /* PARSE_H */
