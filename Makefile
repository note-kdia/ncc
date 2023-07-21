CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

ncc: $(OBJS)
	$(CC) -o ncc $(OBJS) $(LDFLAGS)

$(OBJS): ncc.h

test: ncc
	./test.sh

clean:
	rm -f ncc *.o *~ tmp*

.PHONY: test clean
