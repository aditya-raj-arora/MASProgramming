# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra

# Detect OS (Windows_NT is set on Windows)
ifeq ($(OS),Windows_NT)
    RM = del /Q
    TARGET = mas.exe
    SHELL := cmd.exe
    PATHSEP = \\
else
    RM = rm -f
    TARGET = mas
    SHELL := /bin/sh
    PATHSEP = /
endif

# Source files
SRCS = lexer.c parser.c interpreter.c main.c

# Default target
all: $(TARGET)

# Build target
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^

# Clean target
clean:
	-$(RM) $(TARGET)

.PHONY: all clean
