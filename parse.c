#include "parse.h"

/* Function:	parse()
 *		Given a word, this function will remove unsupported
 *		characters it.  The function also determines whether or
 *		not the word is at the beginning or the end of a sentence -
 *		did we find '. ? ! ,' or nothing?
 */

ENDING parse(char *word) {
	char	*src,
		*dst;
	unsigned len;
	ENDING 	ending;

	assert((len = strlen(word)));
	src = &word[len-1];

	if(*src == '.') 
		ending = PERIOD;
	else if(*src == '!') 
		ending = BANG;
	else if(*src == '?') 
		ending = QUESTION;
	else if(*src == ',')
		ending = COMMA;
	else
		ending = NOTHING;

	if(!strcmp(word, MS) || !strcmp(word, MRS) || !strcmp(word, MR))
		ending = PREFIX;

	src = dst = word;
	while(*src) {
		if(('a' <= *src && *src <= 'z') \
		|| ('A' <= *src && *src <= 'Z') \
		|| *src == '\'' || *src == '-' \
		|| ('0' <= *src && *src <= '9'))
			*dst++ = *src;
		src++;
	}
	*dst = '\0';
	return ending;
}
