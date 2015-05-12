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
static void parse(char *);
static void print_all_nodes(HASH_TABLE *);
static void print_nodes_in_bucket(NODE *);

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
		curr = ht->bucket[i];
		while(curr) {
			prev = curr;
			curr = curr->next;
			rem_node(prev);
		}
		ht->bucket[i] = NULL;
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

static NODE *insert_node(HASH_TABLE *ht, unsigned key, char *word, NODE *prev_node, unsigned is_last) {
	NODE 	*node,
		*prev = NULL;

	printf("bytes in word: %lu\n", strlen(word) + 1);
	if(ht->bucket[key]) {
		node = ht->bucket[key];
		printf("collision '%s' when inserting '%s'\n", ht->bucket[key]->word, word);
		// look for node within table slot 
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
			if(is_last)
				node->last++;
		}
		else {
			prev->next = create_node(key, word, !prev_node ? 1:0, is_last);
			node = prev->next;
		}
	}
	else {
		printf("no collision, inserting '%s'\n", word);
		ht->bucket[key] = create_node(key, word, !prev_node ? 1:0, is_last);
		node = ht->bucket[key];
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
				prev_node->succ = succ;
			}
		}
		prev_node->sum_succ++;
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
	node->sum_succ = 0;
	node->next = NULL;
	node->succ = NULL;
	return node;
}

/* Function: 	print_nodes_in_bucket()
 * Description:	Print all nodes in a slot in the hash table including
 *		their word/key pair, general frequency, freq of being
 *		first and last word in a sentence, and all successors.
 */

static void print_nodes_in_bucket(NODE *node) {
	while(node) {
		printf("WORD: 	'%s'\n", node->word);
		printf("KEY:	'%u'\n", node->key);
		printf("freq:	%u\n", node->freq);
		printf("first:	%u, %%:\t%.3f\n", node->first, (float)node->first/node->freq);
		printf("last:	%u, %%:\t%.3f\n", node->last, (float)node->last/node->freq);
		printf("# succ:	%u\n", node->sum_succ);
		if(node->sum_succ) {
			printf("SUCC:\n");
			SUCC	*temp = node->succ;
			while(temp) {
				printf("\tWORD:\t'%s'\n", temp->node->word);
				printf("\tfreq:\t%u\n", temp->freq);
				printf("\t----\n");
				temp = temp->next;
			}
		}
		node = node->next;
	}
}

/* Function:	print_all_nodes()
 * Description:	Print all nodes in the hash table.
 */

static void print_all_nodes(HASH_TABLE *ht) {
	for(unsigned i = 0; i < USHRT_MAX; i++) {
		if(ht->bucket[i]) {
			print_nodes_in_bucket(ht->bucket[i]);
		}
	}
}

/* Function: 	insert_words()
 * Description:	Doing the dirt work of building the hash table from
 *		a file pointer.  The function scans in words, parses
 *		them, and then inserts them into the table.
 */

void insert_words(HASH_TABLE *ht, FILE *fp) {
	static NODE *node = NULL;
	char 	buf[64];
	unsigned is_last = 0;
	
	while(fscanf(fp, "%s", buf) != EOF) {
		parse(buf);
		node = insert_node(ht, gen_hash(buf), buf, node, is_last);
	}
	
}

/* Function:	parse()
 *		Given a word, this function will remove unsupported
 *		characters it.  The function also determines whether or
 *		not the word is at the beginning or the end of a sentence -
 *		did we find '. ? !'?
 */

// TODO: If we're given a title, ie: Mr., Mrs., Ms., then we don't want to mark end of sentence yet
// TODO: Mark whether it is the end of a sentence and remove the punctuation mark at end of sentence

static void parse(char *word) {
	char	*src,
		*dst;

	src = dst = word;
	while(*src) {
		if('a' <= *src && *src <= 'z')
			*dst++ = *src;
		src++;
	}
	*dst = '\0';
}

int main() {
	char *words[] = {"that's", "a", "lot", "of", "dicks", "that's", "bees"};
	HASH_TABLE *ht;
	NODE 	*node = NULL;
	unsigned is_last = 0;

	ht = create_table();
	for(int i = 0; i < 7; i++) {
		node = insert_node(ht, gen_hash(words[i]), words[i], node, 0);
	}
	printf("clearing nodes\n");
	clear_table(ht);
	//printf("inserting words again\n");
	node = NULL;
	
	for(int i = 0; i < 7; i++) {
		if(i + 1 < 7)
			is_last = 0;
		else
			is_last = 1;
		node = insert_node(ht, 100, words[i], node, is_last);
	}
	print_all_nodes(ht);
	return 1;
}
