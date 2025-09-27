CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2
INCLUDES = -I./include
LIBS = -lSDL3
SRCDIR = src
BUILDDIR = build
TARGET = tsuki

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SOURCES))

ifeq ($(OS),Windows_NT)
    TARGET := $(TARGET).exe
    LIBS += -lmingw32 -lSDL3main
endif

all: $(BUILDDIR) $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LIBS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR) $(TARGET)

run: $(TARGET)
	./$(TARGET)

debug: CFLAGS += -g -DDEBUG
debug: clean $(TARGET)

.PHONY: all clean run debug