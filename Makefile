CC = gcc
CFLAGS = -g
SRC = *.c -lm
OUT = a.out
FILE = test.rs

.PHONY: all run debug vrun clean

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $^ -o $@

run: clean $(OUT)
	./$(OUT) $(FILE)

debug: $(OUT) $(FILE)
	gdb --args ./$(OUT) $(FILE)

vrun: $(OUT)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(OUT) $(FILE)

clean:
	rm -f $(OUT) 
