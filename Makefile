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
LDFLAGS = -L$(RAYLIB_PATH)/bin/Debug -lraylib -lopengl32 -lgdi32 -lwinmm -mwindows



# --- Recreate/Tool Program ---
SOURCES = $(filter-out $(SRCDIR)/shader_main.c, $(wildcard $(SRCDIR)/*.c))
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/recreate_%.o,$(SOURCES))
TARGET = recreate.exe

# Default target (builds the recreate program)
all: $(TARGET)

# --- Build Rules ---



# Build the recreate program
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(BINDIR)/$(TARGET) $(LDFLAGS)



# Compile source files for the recreate program
$(BUILDDIR)/recreate_%.o: $(SRCDIR)/%.c
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@


# --- Run Rules ---

# Run the recreate program
run: all
	@$(BINDIR)/$(TARGET) assets/TARGET.png




# --- Housekeeping ---

# Clean up build files
clean:
	@rm -rf $(BUILDDIR)
	@rm -f $(TARGET) main.exe

.PHONY: all run clean