CC = gcc
CFLAGS = -O3 -Wall -Wextra -g -I./include -I./libs
TARGET = lisp.out

LISP_SRCS = src/val.c src/reader.c src/env.c src/eval.c src/builtin.c src/main.c
LIB_SRCS = libs/linenoise.c

LISP_OBJS = $(LISP_SRCS:.c=.o)
LIB_OBJS = $(LIB_SRCS:.c=.o)
OBJS = $(LISP_OBJS) $(LIB_OBJS)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

libs/%.o: libs/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)