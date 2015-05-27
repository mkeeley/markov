#include "parse.h"

static unsigned starting_apos = 0;

/* Function:	is_ellipsis()
 * Description:	Given a word and whether we are starting from the beginning
 *		or end of the word, determine if we have found an ellpisis (...)
 */

static unsigned is_ellipsis(char *front, char *end, unsigned from_start) {
	unsigned periods = 0;

	if(from_start) 
		while(front != end && *front++ == '.') 
			periods++;
	else 
		while(front != end && *end-- == '.') 
			periods++;
	return periods;
}

/* Function:	parse()
 *		Given a word, this function will remove unsupported
 *		characters it.  The function also determines whether or
 *		not the word is at the beginning or the end of a sentence -
 *		did we find '. ? ! ,' or nothing?
 */

PUNC parse(char *word) {
	char	*dst,
		*front,
		*end;
	unsigned len;
	PUNC	punc;
	memset(&punc, 0, sizeof(punc));

	assert((len = strlen(word)));
	
	// set pointers to beginning and end of current word
	front = word;
	end = &word[len-1];
	
	// check if word begins with quotes or apostrophe
	if(*front == '"') {
		punc.beg_quotes++;
		front++;
	}
	else if(*front == '\'') {
		starting_apos = 1;
		punc.beg_apos++;
		front++;
	}

	// check if word begins with an ellipsis
	unsigned count = is_ellipsis(front, end, START);
	if(count > 2)
		punc.beg_ellipsis++;

	// check if word ends with quotes or apostrophe
	// if we're not within a line of speech (starting_apos is set) then 
	// we assume ending apostrophe does not indicate end of speech
	if(*end == '"') {
		punc.end_quotes++;
		end--;
	}
	else if(*end == '\'' && starting_apos) {
		starting_apos = 0;
		punc.end_apos++;
		end--;
	}

	// check if word ends with an ellipsis
	count = is_ellipsis(front, end, END);
	if(count > 2)
		punc.end_ellipsis++;
	else if(count == 1)
		punc.period++;

	// do we end with !, ?, or a comma?
	if(*end == '!') 
		punc.bang++;
	else if(*end == '?') 
		punc.question++;
	else if(*end == ',')
		punc.comma++;
	else
		punc.nothing++;

	if(!strcmp(word, MS) || !strcmp(word, MRS) || !strcmp(word, MR)) {
		punc.prefix++;
		punc.period = 0;
	}

	front = dst = word;
	while(*front) {
		if(('a' <= *front && *front <= 'z') \
		|| ('A' <= *front && *front <= 'Z') \
		|| *front == '\'' || *front == '-' \
		|| ('0' <= *front && *front <= '9'))
			*dst++ = *front;
		front++;
	}
	*dst = '\0';
	return punc;
}

/* Function:	update_punc()
 * Description:	Given a destination structure of punc, add dest freq to source punc freq
 */

void update_punc(PUNC *dst, PUNC *src) {
	src->nothing += dst->nothing;
	src->comma += dst->comma;
	src->prefix += dst->prefix;
	src->period += dst->period;
	src->question += dst->question;
	src->bang += dst->bang;
	src->beg_quotes += dst->beg_quotes;
	src->end_quotes += dst->end_quotes;
	src->beg_apos += dst->beg_apos;
	src->end_apos += dst->end_apos;
	src->beg_ellipsis += dst->beg_ellipsis;
	src->end_ellipsis += dst->end_ellipsis;
}

