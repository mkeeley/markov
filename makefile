CFLAGS      = -Wall -g
LINKS	    = 
MARKOV_OBJS = markov.o  pcg-c-basic-0.9/pcg_basic.o
MARKOV_PROG = markov 
HASH_OBJS   = hash.o parse.o
HASH_PROG   = hash
PRGS        = $(MARKOV_PROG) $(HASH_PROG)

all:    $(MARKOV_PROG)
	

$(HASH_PROG):	$(HASH_OBJS)
	$(CC) -o $(HASH_PROG) $(HASH_OBJS) $(LINKS)

$(MARKOV_PROG): $(MARKOV_OBJS) $(HASH_OBJS)
	$(CC) -o $(MARKOV_PROG) $(MARKOV_OBJS) $(HASH_OBJS) $(LINKS)

clean:;     $(RM) -f $(PRGS) *.o core
