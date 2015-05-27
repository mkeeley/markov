#ifndef PARSE_H
#define PARSE_H

#include <assert.h>
#include <string.h>

#define MR 	"Mr."
#define MRS	"Mrs."
#define MS	"Ms."
#define START	1
#define END	0

typedef enum {
	NOTHING = 0,
	COMMA 	= 0,
	PREFIX	= 0,
	PERIOD	= 1,
	QUESTION= 1,
	BANG	= 1
} ENDING;

typedef struct {
	unsigned nothing;
	unsigned comma;
	unsigned prefix;
	unsigned period;
	unsigned question;
	unsigned bang;
	unsigned beg_quotes;
	unsigned end_quotes;
	unsigned beg_apos;
	unsigned end_apos;
	unsigned beg_ellipsis;
	unsigned end_ellipsis;
} PUNC;

PUNC parse(char *word);
void update_punc(PUNC *, PUNC *);

#endif /* PARSE_H */
