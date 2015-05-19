CFLAGS      = -Wall 
LINKS	    = 
HASH_OBJS   = hash.o 
MARKOV_OBJS = markov.o 
MARKOV_PROG = markov 
PRGS        = $(MARKOV_PROG)

all:    $(MARKOV_PROG)

$(MARKOV_PROG): $(MARKOV_OBJS) $(HASH_OBJS)
	$(CC) -o $(MARKOV_PROG) $(MARKOV_OBJS) $(HASH_OBJS) $(LINKS)

clean:;     $(RM) -f $(PRGS) *.o core
