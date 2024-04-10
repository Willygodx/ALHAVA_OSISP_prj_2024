CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99
LDLIBS = -lncurses

SRCS = src/file_editor.c
OBJS = $(SRCS:.c=.o)
TARGET = main

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDLIBS) -o $(TARGET)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS) $(TARGET)
