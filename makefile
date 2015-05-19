CFLAGS      = -Wall 
LINKS	    = 
MARKOV_OBJS = markov.o hash.o pcg-c-basic-0.9/pcg_basic.o
MARKOV_PROG = markov 
PRGS        = $(MARKOV_PROG)

all:    $(MARKOV_PROG)
	

$(MARKOV_PROG): $(MARKOV_OBJS) 
	$(CC) -o $(MARKOV_PROG) $(MARKOV_OBJS) $(LINKS)

clean:;     $(RM) -f $(PRGS) *.o core
