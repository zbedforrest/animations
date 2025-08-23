# Makefile for raylib project

# Compiler
CC = gcc

# Project name
TARGET = main

# Source files
SRCDIR = src
SOURCES = $(wildcard $(SRCDIR)/*.c)

# Object files
OBJDIR = build
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))

# Flags
CFLAGS = -Wall -Wextra -I$(SRCDIR) `pkg-config --cflags raylib`
LDFLAGS = `pkg-config --libs raylib`

# Default target
all: $(OBJDIR)/$(TARGET)

# Rule to build the executable
$(OBJDIR)/$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

# Rule to compile source files into object files
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Run the program
run: all
	./$(OBJDIR)/$(TARGET)

# Clean up build files
clean:
	rm -rf $(OBJDIR)/*

# Watch for changes and automatically rebuild and run
watch:
	@echo "Watching for changes in src/... Press Ctrl+C to stop."
	@# This command does the following:
	@# 1. `find src -type f` lists all files in the src directory.
	@# 2. `entr -s '...'` runs the shell script in quotes on any change.
	@# 3. `make all` builds the project. If it fails, the `&&` stops the script, leaving the old app running.
	@# 4. `pkill -f ./build/main || true` kills the old running app. `|| true` prevents an error if it's not found.
	@# 5. `(./build/main &)` starts the new app in the background, allowing entr to continue watching.
	@find $(SRCDIR) -type f | entr -n -s 'make all && (pkill -f ./build/main || true) && (./build/main &)'

.PHONY: all run clean watch
