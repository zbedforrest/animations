# C and raylib Animation Project

This project is a starting point for creating audio-visual animations using C and the `raylib` library.

## Project Structure

-   `src/`: Contains all the C source code (`.c` files).
-   `build/`: This directory is automatically created and will contain the compiled application. It is ignored by Git.
-   `Makefile`: Defines how to compile and run the project.
-   `.gitignore`: Tells Git to ignore the `build/` directory.

## How to Build, Compile, and Run

This project uses a `Makefile` to simplify the development process. You don't need to write complex compiler commands manually.

### 1. Build the Project

To compile your C code and create the executable, open your terminal in the project's root directory and run:

```bash
make
```

This command reads the `Makefile`, compiles all the `.c` files from the `src/` directory, and places the final executable file inside the `build/` directory.

### 2. Run the Application

After a successful build, you can run your application with:

```bash
make run
```

This will execute the compiled program. A window should appear on your screen. To close it, press the `ESC` key or click the window's close button.

### 3. Clean the Build Files

If you want to delete all the compiled files from the `build/` directory, you can run:

```bash
make clean
```

This is useful for starting a fresh build.

## Do I need special compile flags?

Yes, when you use a library like `raylib`, you need to tell the compiler where to find its header files and how to link against its library files. However, the `Makefile` in this project handles this for you automatically using a tool called `pkg-config`.

Here's a breakdown of the flags used in the `Makefile`:

-   `CFLAGS`: These are the "Compiler Flags".
    -   `-Wall -Wextra`: These flags enable extra warnings from the compiler. They are good practice for catching potential bugs in your code.
    -   `-I$(SRCDIR)`: This tells the compiler to look for header files (like `.h` files) inside the `src/` directory.
    -   `` `pkg-config --cflags raylib` ``: This is the most important part for `raylib`. The `pkg-config` tool automatically finds the correct compiler flags needed for `raylib` on your system. This makes your project more portable.

-   `LDFLAGS`: These are the "Linker Flags".
    -   `` `pkg-config --libs raylib` ``: This command gets the necessary flags to "link" your program with the `raylib` library files, which contain the actual `raylib` functions.

Because the `Makefile` is already set up with these commands, you don't have to worry about managing these flags yourself. You can just focus on writing your C code and use the simple `make` commands to handle the rest.
