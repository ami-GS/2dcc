CFLAGS= -std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)


.PHONY: test clean

2dcc: $(OBJS)
	$(CC) -o 2dcc $(OBJS) $(LDFLAGS)

$(OBJS): 2dcc.h

test: 2dcc
	./tests.sh

clean:
	rm -f 2dcc *.o *.s *~ tmp*

