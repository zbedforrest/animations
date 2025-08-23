# Makefile for raylib project

# Compiler
CC = gcc

# Project names
TARGET_MAIN = main
TARGET_ANALYZER = image_analyzer

# Source files
SRCDIR = src
SRC_MAIN = $(SRCDIR)/main.c
SRC_ANALYZER = $(SRCDIR)/image_analyzer.c

# Object files
OBJDIR = build
OBJ_MAIN = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRC_MAIN))
OBJ_ANALYZER = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRC_ANALYZER))

# Flags
CFLAGS = -Wall -Wextra -I$(SRCDIR) `pkg-config --cflags raylib`
LDFLAGS = `pkg-config --libs raylib`

# Default target
all: $(OBJDIR)/$(TARGET_MAIN)

# Rule to build the main executable
$(OBJDIR)/$(TARGET_MAIN): $(OBJ_MAIN)
	$(CC) $(OBJ_MAIN) -o $@ $(LDFLAGS)

# Rule to build the analyzer executable
analyzer: $(OBJDIR)/$(TARGET_ANALYZER)

$(OBJDIR)/$(TARGET_ANALYZER): $(OBJ_ANALYZER)
	$(CC) $(OBJ_ANALYZER) -o $@ $(LDFLAGS)

# Rule to compile source files into object files
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Run the program
run: all
	./$(OBJDIR)/$(TARGET_MAIN)

# Run the analyzer
run_analyzer: analyzer
	./$(OBJDIR)/$(TARGET_ANALYZER)

# Clean up build files
clean:
	rm -rf $(OBJDIR)/*

# Watch for changes and automatically rebuild and run
watch:
	@echo "Watching for changes in src/... Press Ctrl+C to stop."
	@find $(SRCDIR)/main.c $(SRCDIR)/glow_ring.fs | entr -n -s 'make all && (pkill -f ./build/main || true) && (./build/main &)'

.PHONY: all run clean watch analyzer run_analyzer