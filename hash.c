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
static void add_succ(NODE *, NODE *); 
static unsigned parse(char *);
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

	//printf("bytes in word: %lu\n", strlen(word) + 1);
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
	if(!prev_node)
		ht->sentences++;
	ht->count++;
	add_succ(prev_node, node);
	return node;
}

/* Function:	add_succ()
 * Description:	Add the current node to previous node's list of
 *		successors and update frequency.
 */

static void add_succ(NODE *prev_node, NODE *node) {
	SUCC	*curr_s,
		*prev_s,
		*succ;

	if(prev_node) {
		//printf("adding '%s' to '%s'->succ\n", node->word, prev_node->word);
		curr_s = prev_node->succ;
		prev_s = NULL;
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
				prev_s->next = succ;
			}
			// if list is empty
			else {
				prev_node->succ = succ;
			}
			prev_node->num_succ++;
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
	node->num_succ = 0;
	node->traversed = 0;
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
	SUCC	*temp;

	while(node) {
		printf("WORD: 	'%s'\n", node->word);
		printf("KEY:	'%u'\n", node->key);
		printf("freq:	%u\n", node->freq);
		printf("first:	%u, %%:\t%.3f\n", node->first, (float)node->first/node->freq);
		printf("last:	%u, %%:\t%.3f\n", node->last, (float)node->last/node->freq);
		printf("succfrq:%u\n", node->sum_succ);
		printf("succnum:%u\n", node->num_succ);
		if(node->sum_succ) {
			printf("SUCC:\n");
			temp = node->succ;
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
	
	while(fscanf(fp, "%s", buf) != EOF) {
		is_last = parse(buf);
		node = insert_node(ht, gen_hash(buf), buf, node, is_last);
		// if last word in sentence, reset node pointer and is_last flag
		// node == NULL flags insert_node that next word is first in sentence
		if(is_last) {
			node = NULL;
			is_last = 0;
		}
	}
	
}

/* Function:	parse()
 *		Given a word, this function will remove unsupported
 *		characters it.  The function also determines whether or
 *		not the word is at the beginning or the end of a sentence -
 *		did we find '. ? !'?
 */

static unsigned parse(char *word) {
	char	*src,
		*dst;
	unsigned len,
		 is_last = 0;

	len = strlen(word);
	if(len && (word[len-1] == '.' || word[len-1] == '!' || word[len-1] == '?')) 
		is_last = 1;
	if(!strcmp(word, MS) || !strcmp(word, MRS) || !strcmp(word, MR))
		is_last = 0;
	src = dst = word;
	while(*src) {
		//*src = tolower(*src);
		if(('a' <= *src && *src <= 'z') || ('A' <= *src && *src <= 'Z') || *src == '\'' || *src == '-' || ('0' <= *src && *src <= '9'))
			*dst++ = *src;
		src++;
	}
	*dst = '\0';
	return is_last;
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
