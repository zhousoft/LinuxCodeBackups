.PHONY:clean
CC=g++
CFLAGS=-Wall -g
BIN=cgi.out
OBJS=main.o 

$(BIN):$(OBJS)
	$(CC) $(CFLAGS) $^ -o $@
$(OBJS):main.c processpool.h
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f *.o $(BIN)
