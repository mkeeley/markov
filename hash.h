#ifndef HASH_H
#define HASH_H

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <string.h>

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
	unsigned sum_succ;
	char	*word;
	struct node *next;
	SUCC	*succ;
} NODE;
	
typedef struct {
	unsigned count;
	unsigned sentences;
	NODE	*table[USHRT_MAX];
} HASH_TABLE;

#endif /* HASH_H */


