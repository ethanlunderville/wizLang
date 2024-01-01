CC = gcc
CFLAGS = -g
SRC = *.c -lm
OUT = a.out

.PHONY: all run debug vrun clean

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $^ -o $@

run: clean $(OUT)
	./$(OUT)

debug: $(OUT)
	gdb ./$(OUT)

vrun: $(OUT)
	valgrind ./$(OUT)

clean:
	rm -f $(OUT)
