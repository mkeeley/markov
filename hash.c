#include "hash.h"

/* Author: 	Mickey Keeley
 * File:	hash.c
 * Description:	Implement hash ht with linear collision 
 *		resolution using the 16-bit FNV-1a hash function
 */

#define OFFSET	2166136261
#define PRIME	16777619
#define MASK	((1 << 16) - 1) // 0xFFFF

static unsigned gen_hash(char *);
static NODE *create_node(unsigned, char *, unsigned, unsigned);
static NODE *insert_node(HASH_TABLE *, unsigned, char *, NODE *, unsigned);
static HASH_TABLE *create_table();
static HASH_TABLE *clear_table(HASH_TABLE *);
static void rem_node(NODE *);
static void rem_table(HASH_TABLE *);
static void add_succ(NODE *, NODE *); 

/* Function: 	gen_hash()
 * Description:	Generate 16-bit hash value for a given input string.
 *		For every byte, XOR with hash value, multiply by the
 *		prime, and then shift and apply the mask to get 16 bits.
 */

static unsigned gen_hash(char *s) {
	unsigned hash = OFFSET;
	while(*s) {
		hash ^= *s++;
		hash *= PRIME;
	}
	hash = (hash >> 16) ^ (hash & MASK);
	return hash;
}
		
/* Function:	create_table() 
 * Description:	Create and return a hash table. 'count' keeps track of the total
 *		number of words in the table and 'sentences' keeps track of the
 *		total number of sentences in the table.
 */

static HASH_TABLE *create_table() {
	HASH_TABLE *ht = malloc(sizeof(HASH_TABLE));
	assert(ht);
	
	ht->count = 0;
	ht->sentences = 0;
	return ht;
}
	
/* Function: 	clear_table()
 * Description:	Remove all nodes from the hash table, reset 'count' and 'sentences.'
 *		Return an empty table, the table has not been freed.
 */

static HASH_TABLE *clear_table(HASH_TABLE *ht) {
	unsigned i;
	NODE	*curr,
		*prev;

	for(i = 0; i < USHRT_MAX; i++) {
		curr = ht->table[i];
		while(curr) {
			prev = curr;
			curr = curr->next;
			rem_node(prev);
		}
		ht->table[i] = NULL;
	}
	ht->count = 0;
	ht->sentences = 0;
	return ht;
}

/* Function:	rem_node() 
 * Description:	Remove node, free 'node' and all elements in 'succ.'
 */

void rem_node(NODE *node) {
	SUCC 	*curr = node->succ,
		*prev = NULL;

	while(curr) {
		prev = curr;
		curr = curr->next;
		free(prev);
	}
	printf("key '%u', freeing '%s'\n", node->key, node->word);
	free(node->word);
	free(node);
	node = NULL;
}

/* Function:	rem_table()
 * Description:	Free the hash table.
 */

void rem_table(HASH_TABLE *ht) {
	free(ht);
}

/* Function:	insert_node()
 * Description:	Inserts the key/word pair into the hash table and returns the current node
 *		to use as next function call's prev_node
 */

static NODE *insert_node(HASH_TABLE *ht, unsigned key, char *word, NODE *prev_node, unsigned next_key) {
	NODE 	*node,
		*prev = NULL;

	printf("bytes in word: %lu\n", strlen(word) + 1);
	if(ht->table[key]) {
		node = ht->table[key];
		printf("collision '%s' when inserting '%s'\n", ht->table[key]->word, word);
		// look for node within table slot -> should move to own function, will be doing multiple times
		while(node && strcmp(node->word, word)) {
			prev = node;
			node = node->next;
		}
		// if node already inserted
		if(node) {
			printf("already found '%s,' updating freq of node\n", word);
			node->freq++;
			// no prev_node means first in sentence
			if(!prev_node)
				node->first++;
			// no next_key means last in sentence
			if(!next_key)
				node->last++;
		}
		else {
			prev->next = create_node(key, word, !prev_node ? 1:0, !next_key ? 1:0);
			node = prev->next;
		}
	}
	else {
		printf("no collision, inserting '%s'\n", word);
		ht->table[key] = create_node(key, word, !prev_node ? 1:0, !next_key ? 1:0);
		node = ht->table[key];
	}
	ht->count++;
	add_succ(prev_node, node);
	return node;
}

/* Function:	add_succ()
 * Description:	Add the current node to previous node's list of
 *		successors and update frequency.
 */

static void add_succ(NODE *prev_node, NODE *node) {
	// node already exists, search through prev_node's succ list and increase freq or add word
	if(prev_node) {
		printf("adding '%s' to '%s'->succ\n", node->word, prev_node->word);
		SUCC	*curr_s = prev_node->succ,
			*prev_s = NULL,
			*succ;
		while(curr_s && strcmp(curr_s->node->word, node->word)) {
			prev_s = curr_s;
			curr_s = curr_s->next;
		}
		// succ exists
		if(curr_s) {
			curr_s->freq++;
		}
		else {
			succ = malloc(sizeof(*succ));
			succ->next = NULL;
			succ->node = node;
			succ->freq = 1;
			// if succ does not exist and list is not empty
			if(prev_s) {	
				printf("adding!\n");
				prev_s->next = succ;
				printf("added!\n");
			}
			// if list is empty
			else {
				curr_s = succ;
			}
		}
		prev_node->total_succ++;
	}
}

/* Function: 	create_node()
 * Description:	Creates a node with the given key/word pair.  It will be known whether or not the
 *		word was the first or last in a sentence.
 */

static NODE *create_node(unsigned key, char *word, unsigned is_first, unsigned is_last) {
	NODE 	*node = malloc(sizeof(*node));
	assert(node);

	node->word = malloc(strlen(word) + 1);
	strcpy(node->word, word);
	node->key = key;
	node->freq = 1;
	node->first = is_first;
	node->last = is_last;
	node->total_succ = 0;
	node->next = NULL;
	node->succ = NULL;
	return node;
}


int main() {
	char *words[] = {"that's", "a", "lot", "of", "dicks", "that's"};
	HASH_TABLE *ht;
	NODE 	*node = NULL;

	ht = create_table();
	for(int i = 0; i < 6; i++) {
		node = insert_node(ht, gen_hash(words[i]), words[i], node, 0);
	}
	printf("clearing nodes\n");
	clear_table(ht);
	//printf("inserting words again\n");
	for(int i = 0; i < 6; i++) {
		node = insert_node(ht, gen_hash(words[i]), words[i], node, 0);
	}
	return 1;
}
