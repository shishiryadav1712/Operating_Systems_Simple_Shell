# Makefile for compiling and running shell.c

# Compiler to use
CC = gcc

# Compiler flags:
# -Wall      : Enable all warning messages
# -Wextra    : Enable extra warning flags
# -std=c99   : Use the C99 standard
# -D_POSIX_C_SOURCE=200809L : Enable POSIX features
CFLAGS = -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=200809L

# Target executable name
TARGET = shell

# Source files
SRCS = shell.c

# Object files
OBJS = $(SRCS:.c=.o)

# Default target to build the executable
all: $(TARGET)

# Link the object files to create the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Run the shell executable
run: $(TARGET)
	./$(TARGET)

# Clean up build files
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets to avoid conflicts with files named 'all', 'run', or 'clean'
.PHONY: all run clean
