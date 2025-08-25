# Makefile for raylib project on Windows

# Compiler
CC = gcc

# Directories
SRCDIR = src
INCDIR = include
BUILDDIR = build
BINDIR = .

# Raylib path
RAYLIB_PATH = ../raylib-quickstart
RAYLIB_HEADER_PATH = $(RAYLIB_PATH)/build/external/raylib-master/src

# Flags
CFLAGS = -Wall -Wextra -g -I$(INCDIR) -I$(RAYLIB_HEADER_PATH)
LDFLAGS = -L$(RAYLIB_PATH)/bin/Debug -lraylib -lopengl32 -lgdi32 -lwinmm -static -mwindows

# --- Shader Program ---
SHADER_SOURCES = $(filter-out $(SRCDIR)/main.c, $(wildcard $(SRCDIR)/*.c))
SHADER_OBJECTS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SHADER_SOURCES))
SHADER_TARGET = main.exe

# --- Recreate/Tool Program ---
RECREATE_SOURCES = $(filter-out $(SRCDIR)/shader_main.c, $(wildcard $(SRCDIR)/*.c))
RECREATE_OBJECTS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/recreate_%.o,$(RECREATE_SOURCES))
RECREATE_TARGET = recreate.exe

# Default target (builds the shader program)
all: $(SHADER_TARGET)

# --- Build Rules ---

# Build the shader program
$(SHADER_TARGET): $(SHADER_OBJECTS)
	$(CC) $(SHADER_OBJECTS) -o $(BINDIR)/$(SHADER_TARGET) $(LDFLAGS)

# Build the recreate program
$(RECREATE_TARGET): $(RECREATE_OBJECTS)
	$(CC) $(RECREATE_OBJECTS) -o $(BINDIR)/$(RECREATE_TARGET) $(LDFLAGS)

# Compile source files for the shader program
$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile source files for the recreate program
$(BUILDDIR)/recreate_%.o: $(SRCDIR)/%.c
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@


# --- Run Rules ---

# Run the shader program
run: all
	@$(BINDIR)/$(SHADER_TARGET)

# Target to build and run the recreate program
run_recreate: $(RECREATE_TARGET)
	@$(BINDIR)/$(RECREATE_TARGET) assets/TARGET.png


# --- Housekeeping ---

# Clean up build files
clean:
	@if exist $(BUILDDIR) rd /s /q $(BUILDDIR)
	@if exist $(SHADER_TARGET) del $(SHADER_TARGET)
	@if exist $(RECREATE_TARGET) del $(RECREATE_TARGET)

.PHONY: all run run_recreate clean