CC=g++ 
CFLAGS=-Wall -g
program := h264_parser
SRCS := $(wildcard *.cpp)
OBJS := ${SRCS:.cpp=.o}

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

