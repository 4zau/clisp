CC = gcc
CFLAGS = -Wall -Wextra -g -I./include -I./libs
TARGET = lisp

all: $(TARGET)

$(TARGET): libs/linenoise.c src/val.c src/reader.c src/env.c src/eval.c src/builtin.c src/main.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(TARGET)