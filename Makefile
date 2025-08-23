# Makefile for raylib project

# Compiler
CC = gcc

# Directories
SRCDIR = src
INCDIR = include
OBJDIR = build
BINDIR = bin
ASSETDIR = assets

# Project names
TARGET_SHADER = main
TARGET_TOOL = image_tool

# Source files
TOOL_SOURCES = $(filter-out $(SRCDIR)/shader_main.c, $(wildcard $(SRCDIR)/*.c))
SHADER_SRC = $(SRCDIR)/shader_main.c

# Object files
TOOL_OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(TOOL_SOURCES))
SHADER_OBJECT = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SHADER_SRC))

# Executables
TOOL_EXEC = $(BINDIR)/$(TARGET_TOOL)
SHADER_EXEC = $(BINDIR)/$(TARGET_SHADER)

# Flags
CFLAGS = -Wall -Wextra -I$(INCDIR) `pkg-config --cflags raylib`
LDFLAGS = `pkg-config --libs raylib`

# Default target - build the tool
all: $(TOOL_EXEC)

# Rule to build the shader executable
shader: $(SHADER_EXEC)

$(SHADER_EXEC): $(SHADER_OBJECT)
	@mkdir -p $(BINDIR)
	$(CC) $(SHADER_OBJECT) -o $@ $(LDFLAGS)

# Rule to build the tool executable
tool: $(TOOL_EXEC)

$(TOOL_EXEC): $(TOOL_OBJECTS)
	@mkdir -p $(BINDIR)
	$(CC) $(TOOL_OBJECTS) -o $@ $(LDFLAGS)

# Rule to compile source files into object files
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Run the shader
run_shader: shader
	$(SHADER_EXEC)

# Run the tool
run: tool
	$(TOOL_EXEC) $(ASSETDIR)/TARGET.png

# Clean up build files
clean:
	rm -rf $(OBJDIR)/* $(BINDIR)/*

.PHONY: all shader tool run_shader run clean
