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
static void rem_node(NODE *);
static SUCC *add_succ(SUCC *, NODE *); 
static void print_nodes_in_bucket(NODE *);
static PREC *add_prec(NODE *, PREC *);
static SUCC *find_succ(NODE *, SUCC *);

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

HASH_TABLE *create_table() {
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

HASH_TABLE *clear_table(HASH_TABLE *ht) {
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
	PREC 	*curr_p = node->prec,
		*prev_p = NULL;
	SUCC	*curr_s,
		*prev_s = NULL;

	while(curr_p) {
		curr_s = curr_p->succ;
		while(curr_s) {
			prev_s = curr_s;
			curr_s = curr_s->next;
			free(prev_s);
		}
		prev_p = curr_p;
		curr_p = curr_p->next;
		free(prev_p);
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
// TODO: split into multiple functions, currently is cumbersome pos

static NODE *insert_node(HASH_TABLE *ht, unsigned key, char *word, NODE *prev_node, unsigned is_last) {
	static PREC *prev_prec = NULL;
	static unsigned prev_was_first = 0;
	PREC	*temp;
	NODE 	*node,
		*prev = NULL;

	//printf("adding %s to table\n", word);

	// TABLE INSERTION
	if(ht->bucket[key]) {
		node = ht->bucket[key];
		//printf("collision '%s' when inserting '%s'\n", ht->bucket[key]->word, word);
		// look for node within table slot 
		while(node && strcmp(node->word, word)) {
			prev = node;
			node = node->next;
		}
		// if node already inserted
		if(node) {
			//printf("already found '%s,' updating freq of node\n", word);
			node->freq++;
			// no prev_node means first in sentence
			if(!prev_node) {
				node->first++;
			}
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
		//printf("no collision, inserting '%s'\n", word);
		ht->bucket[key] = create_node(key, word, !prev_node ? 1:0, is_last);
		node = ht->bucket[key];
	}

	// PREC INSERTION
	if(prev_node) {
		// update freq of prec, if does not exist, insert at head. set temp to prec node for next section
		temp = find_prec(prev_node, node);
		if(!temp) {
			node->prec = add_prec(prev_node, node->prec);
			node->sum_prec++;
		}
		node->num_prec++;

		// check if prev word was first in sentence, if so then add to list of succ for prev_node
		if(prev_was_first) {
			SUCC	*succ = find_succ(node, prev_node->succ);

			if(succ) {
				succ->freq++;
			}
			else {
				prev_node->succ = add_succ(prev_node->succ, node);
				prev_node->num_succ++;
			}
			prev_was_first = 0;
			prev_node->sum_succ++;
		}
	}
	else {
		ht->sentences++;
		prev_was_first = 1;
	}

	// SUCC INSERTION
	// prev_prec shold be equal to head of the previous-prev_node's prec list
	if(prev_prec) {
		SUCC	*curr = find_succ(node, prev_prec->succ);

		if(curr) {
			curr->freq++;
		}
		else {
			prev_prec->succ = add_succ(prev_prec->succ, node);
			prev_prec->num_succ++;
		}
		prev_prec->sum_succ++;
	}
	// set new prec pointer if prev_node exists, must come after setting prev_prec's succ
	if(prev_node) {
		prev_prec = find_prec(prev_node, node);
	}
	// if last word in sentence, reset prev pointer, there is no preceeding word before start of sentence
	if(is_last)
		prev_prec = NULL;
	ht->count++;

	return node;
}

/* Function:	add_prec()
 * Description:	Add new prec to head of list of preceeding words (nodes).
 */

static PREC *add_prec(NODE *prev_node, PREC *prec) {
	PREC *new = malloc(sizeof(*new));
	new->node = prev_node;
	new->freq = 1;
	new->sum_succ = 0;
	new->num_succ = 0;
	new->next = prec;
	new->succ = NULL;
	
	return new;
}
	
/* Function:	find_prec()
 * Description:	Given a node, check if prev_node exists in node's list of preceeding
 *		nodes.
 */

PREC *find_prec(NODE *prev_node, NODE *node) {
	PREC	*curr,
		*prev = NULL;
	
	curr = node->prec;
	while(curr && curr->node != prev_node) {
		prev = curr;
		curr = curr->next;
	}
	return curr;
}

/* Function:	add_succ()
 * Description:	Add the current node to previous node's list of
 *		successors and update frequency.
 */

static SUCC *add_succ(SUCC *head, NODE *node) {
	SUCC	*succ;

	//printf("adding '%s' to '%s'->succ\n", node->word, prev_node->word);
	succ = malloc(sizeof(*succ));
	succ->next = head;
	succ->node = node;
	succ->freq = 1;
	return succ;
}

/* Function:	find_succ()
 * Description:	Given a pointer to list of successors, find the node in the given list. If not found,
 *		then return NULL.
 */

static SUCC *find_succ(NODE *needle, SUCC *haystack) {
	SUCC	*curr = haystack,
		*prev = NULL;

	while(curr && curr->node != needle) {
		prev = curr;
		curr = curr->next;
	}
	return curr;
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
	node->sum_prec = 0;
	node->num_prec = 0;
	node->sum_succ = 0;
	node->num_succ = 0;
	node->next = NULL;
	node->prec= NULL;
	node->succ = NULL;
	node->punc = malloc(sizeof(PUNC));
	return node;
}

/* Function: 	print_nodes_in_bucket()
 * Description:	Print all nodes in a slot in the hash table including
 *		their word/key pair, general frequency, freq of being
 *		first and last word in a sentence, and all successors.
 */

static void print_nodes_in_bucket(NODE *node) {
	PREC	*prec;
	SUCC	*succ;

	while(node) {
		printf("WORD: 	'%s'\n", node->word);
		printf("KEY:	'%u'\n", node->key);
		printf("freq:	%u\n", node->freq);
		printf("first:	%u, %%:\t%.3f\n", node->first, (float)node->first/node->freq);
		printf("last:	%u, %%:\t%.3f\n", node->last, (float)node->last/node->freq);
		printf("precfrq:%u\n", node->sum_prec);
		printf("precnum:%u\n", node->num_prec);
		if(node->first) {
			succ = node->succ;
			printf("WAS FIRST?:\n");
			while(succ) {
				printf("\t\tWORD:\t'%s'\n", succ->node->word);
				printf("\t\tfreq:\t%u\n", succ->freq);
				printf("\t\t----\n");
				succ = succ->next;
			}
		}
		if(node->sum_prec) {
			printf("PREC:\n");
			prec = node->prec;
			while(prec) {
				printf("\tWORD:\t'%s'\n", prec->node->word);
				printf("\tfreq:\t%u\n", prec->freq);
				printf("\t----\n");
				succ = prec->succ;
				while(succ) {
					printf("\t\tWORD:\t'%s'\n", succ->node->word);
					printf("\t\tfreq:\t%u\n", succ->freq);
					printf("\t\t----\n");
					succ = succ->next;
				}
				prec = prec->next;
			}
		}
		node = node->next;
	}
}

/* Function:	print_all_nodes()
 * Description:	Print all nodes in the hash table.
 */

void print_all_nodes(HASH_TABLE *ht) {
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
	PUNC punc;
	
	while(fscanf(fp, "%s", buf) != EOF) {
		memset(&punc, 0, sizeof(punc));
		punc = parse(buf);
		is_last = punc.period | punc.question | punc.bang;
		node = insert_node(ht, gen_hash(buf), buf, node, is_last);
		update_punc(node->punc, &punc);
		// if last word in sentence, reset node pointer and is_last flag
		// node == NULL flags insert_node that next word is first in sentence
		if(is_last) {
			node = NULL;
			is_last = 0;
		}
	}
	
}

/* Function:	get_next_node()
 * Description:	Return the next valid node from the hash table.  It goes through
 *		each node in the current bucket before moving to the next one. 
 *		This function should be called until it returns NULL, effectively 
 *		resetting the pointer within the function.
 */

NODE *get_next_node(HASH_TABLE *ht) {
	static HASH_TABLE *local_ht = NULL;
	static NODE *local_node = NULL;
	static unsigned i = 0;
	
	assert(ht);
	if(ht != local_ht || i == USHRT_MAX) {
		//printf("resetting %s local vars\n", __FUNCTION__);
		local_ht = ht;
		local_node = NULL;
		i = 0;
	}

	do {
		if(local_node)
			local_node = local_node->next;
		else 
			local_node = local_ht->bucket[i++];
	} while(!local_node && i < USHRT_MAX);
	return local_node;
}

/* Function:	get_sentences()
 * Description:	Return the number of sentences in the hash table.
 */

unsigned get_sentences(HASH_TABLE *ht) {
	assert(ht);
	return ht->sentences;
}
/*
int main() {
	HASH_TABLE *ht = create_table();
	FILE *fp = fopen("test.txt", "r");
	insert_words(ht, fp);
	print_all_nodes(ht);
	
	return 1;
}
*/
