CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude -lm
TARGET = kappok
SOURCES = src/main.c src/lexer.c src/parser.c src/interpreter.c
HEADERS = include/kappok.h
VPATH = src:include

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

.PHONY: all clean
