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
	unsigned freq;
	struct node *node;
	struct succ *next;
} SUCC;

typedef struct node {
	unsigned key;
	unsigned first;
	unsigned last;
	unsigned freq;
	unsigned sum_succ;	// total freq of all succ nodes
	unsigned num_succ;	// total number of unique succ nodes
	struct node *next;
	char	*word;
	SUCC	*succ;
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


