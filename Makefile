CFLAGS= -std=c11 -g -static

.PHONY: test clean

2dcc: 2dcc.c

test: 2dcc
	./tests.sh

clean:
	rm -f 2dcc *.o *.s *~ tmp*

