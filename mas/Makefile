# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra

# Target executable
TARGET = mas.exe

# Source files
SRCS = lexer.c parser.c interpreter.c main.c

# Default target
all: $(TARGET)

# Link object files into executable
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^

# Clean generated files
clean:
	del $(TARGET) 2> NUL || rm -f $(TARGET)

.PHONY: all clean