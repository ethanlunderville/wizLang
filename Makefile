CC = gcc
CFLAGS = -g
SRC = *.c -lm
OUT = a.out

.PHONY: all run debug vrun clean

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $^ -o $@

run: clean $(OUT)
	./$(OUT) test.rs

debug: $(OUT)
	gdb ./$(OUT)

vrun: $(OUT)
	valgrind --leak-check=full --show-leak-kinds=all ./$(OUT)

clean:
	rm -f $(OUT)
