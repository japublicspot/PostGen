CC= clang
PROG= ./bin/interPS
CFLAGS= -g -Wall
OBJS= ./src/main.o ./src/eval.o

all: $(PROG)

$(PROG): $(OBJS)
	mkdir -p ./bin
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS) -lm

clean:
	rm -f $(PROG) $(OBJS)
