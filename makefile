CC = gcc
CFLAGS = -g -Wall
LIBS = -lreadline
SRC = src/main.c src/lsh.c src/trie.c
OUT = lsh

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LIBS)

clean:
	rm -f $(OUT) a.out

run: $(OUT)
	./$(OUT)