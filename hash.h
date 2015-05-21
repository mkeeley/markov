#ifndef HASH_H
#define HASH_H

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#define MR 	"Mr."
#define MRS	"Mrs."
#define MS	"Ms."

typedef struct succ {
	struct node *node;	// node of the successor
	struct succ *next;	// the next successor
	unsigned freq;		// occurrences
} SUCC;

typedef struct prec {
	struct node *node;	// preceeding node
	struct succ *succ;	// list of successors
	struct prec *next;	// the next preceeding node
	unsigned sum_succ;	// total occurrences of successors
	unsigned num_succ;	// number of unique successors
	unsigned freq;		// occurrences
} PREC;

typedef struct node {
	unsigned key;		// hash value
	unsigned first;		// num times word is first in sentence
	unsigned last;		// num times word is last in setence
	unsigned freq;		// num times word occurs
	unsigned sum_prec;	// total freq of all succ nodes
	unsigned num_prec;	// total number of unique succ nodes
	struct node *next;	// collision: next node
	char	*word;		// hashed word
	PREC	*prec;		// list of preceeding words (nodes)
	SUCC	*succ;		// ONLY FOR BEGINNING OF SENTENCES, need to know which words follow
	unsigned traversed;	// penalty for words that have been chosen already - reduce loops and over reuse of more probable words
} NODE;
	
typedef struct {
	unsigned count;
	unsigned sentences;
	NODE	*bucket[USHRT_MAX];
} HASH_TABLE;

HASH_TABLE *create_table();
HASH_TABLE *clear_table(HASH_TABLE *);
NODE *get_next_node(HASH_TABLE *);
void insert_words(HASH_TABLE *, FILE *);
void print_all_nodes(HASH_TABLE *);
void rem_table(HASH_TABLE *);
unsigned get_sentences(HASH_TABLE *);

#endif /* HASH_H */


