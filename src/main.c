#include "raylib.h"
#include "app.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s <image_file>\n", argv[0]);
        return 1;
    }

    const int screenWidth = 1400;
    const int screenHeight = 800;
    InitWindow(screenWidth, screenHeight, "Image Tool");
    SetTargetFPS(60);

    AppState state = { 0 };
    InitApp(&state, argv[1]);
    RunApp(&state);
    CleanupApp(&state);

    CloseWindow();

    return 0;
}



