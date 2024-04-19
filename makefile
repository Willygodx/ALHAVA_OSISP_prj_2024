CC = gcc
CFLAGS =-W -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -std=c11 -pedantic
LDLIBS = -lncurses

SRCDIR = src
OBJDIR = obj

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))
TARGET = file_editor

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDLIBS) -o $(TARGET)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(OBJDIR)/.dummy
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/.dummy:
	mkdir -p $(OBJDIR)
	touch $(OBJDIR)/.dummy

clean:
	$(RM) -r $(OBJDIR) $(TARGET)
