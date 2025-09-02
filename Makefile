# Makefile for raylib project

# Compiler
CC = gcc

# Directories
SRCDIR = src
INCDIR = include
BUILDDIR = build
BINDIR = .

# OS detection
ifeq ($(shell uname -s),Darwin)
    # macOS settings
    RAYLIB_PATH = raylib
    RAYLIB_LIB_PATH = $(RAYLIB_PATH)/src
    CFLAGS = -Wall -Wextra -g -I$(INCDIR) -I$(RAYLIB_LIB_PATH) -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL
    LDFLAGS = -L$(RAYLIB_LIB_PATH) -lraylib -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL
    TARGET = recreate
    CLEAN_TARGET = $(TARGET)
else
    # Windows settings
    RAYLIB_PATH = ../raylib-quickstart
    RAYLIB_HEADER_PATH = $(RAYLIB_PATH)/build/external/raylib-master/src
    CFLAGS = -Wall -Wextra -g -I$(INCDIR) -I$(RAYLIB_HEADER_PATH)
    LDFLAGS = -L$(RAYLIB_PATH)/bin/Debug -lraylib -lopengl32 -lgdi32 -lwinmm -mwindows
    TARGET = recreate.exe
    CLEAN_TARGET = $(TARGET) main.exe
endif

# --- Recreate/Tool Program ---
SOURCES = $(filter-out $(SRCDIR)/shader_main.c, $(wildcard $(SRCDIR)/*.c))
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/recreate_%.o,$(SOURCES))

# Default target (builds the recreate program)
all: $(TARGET)

# --- Build Rules ---

# Build the recreate program
ifeq ($(shell uname -s),Darwin)
$(TARGET): $(OBJECTS) $(RAYLIB_LIB_PATH)/libraylib.a
	$(CC) $(OBJECTS) -o $(BINDIR)/$(TARGET) $(LDFLAGS)

$(RAYLIB_LIB_PATH)/libraylib.a:
	@$(MAKE) -C $(RAYLIB_PATH)/src
else
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(BINDIR)/$(TARGET) $(LDFLAGS)
endif

# Compile source files for the recreate program
$(BUILDDIR)/recreate_%.o: $(SRCDIR)/%.c
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# --- Run Rules ---

# Run the recreate program
run: all
	@$(BINDIR)/$(TARGET) assets/TARGET5.png

# --- Housekeeping ---

# Clean up build files
clean:
	@rm -rf $(BUILDDIR)
	@rm -f $(CLEAN_TARGET)
ifeq ($(shell uname -s),Darwin)
	@$(MAKE) -C $(RAYLIB_PATH)/src clean
endif

.PHONY: all run clean