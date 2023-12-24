CC = gcc
CFLAGS = -g
SRC = Compile.c AST.c Parse.c Interpreter.c Codegen.c Error.c -lm
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
