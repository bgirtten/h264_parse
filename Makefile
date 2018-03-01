CC=gcc 
CFLAGS=-Wall -g
program := utilstest
SRCS := $(wildcard *.c)
OBJS := ${SRCS:.c=.o}

.PHONY: all clean distclean run

all: $(program)

$(program): $(OBJS)
	$(CC) $(OBJS) -o $(program)

clean:
	@- $(RM) $(program)
	@- $(RM) $(OBJS)

distclean: clean
run: all
	valgrind --leak-check=full --show-reachable=yes ./$(program)

