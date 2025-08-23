#include "raylib.h"

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 300;
    const int screenHeight = 200;

    SetConfigFlags(FLAG_MSAA_4X_HINT); // Enable 4x MSAA
    InitWindow(screenWidth, screenHeight, "Glowing Ring Shader");

    // Position the window on the right side of the primary monitor
    int monitor = GetCurrentMonitor();
    int monitorWidth = GetMonitorWidth(monitor);
    SetWindowPosition(monitorWidth - screenWidth - 50, 50); // 50px padding from right and top

    // Load the shader
    Shader shader = LoadShader(0, TextFormat("src/glow_ring.fs", GLSL_VERSION));

    // Get shader uniform locations
    int timeLoc = GetShaderLocation(shader, "u_time");
    int resolutionLoc = GetShaderLocation(shader, "u_resolution");

    float resolution[2] = { (float)screenWidth, (float)screenHeight };
    SetShaderValue(shader, resolutionLoc, resolution, SHADER_UNIFORM_VEC2);

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())
    {
        // Update
        //----------------------------------------------------------------------------------
        // Pass the elapsed time to the shader
        float time = (float)GetTime();
        SetShaderValue(shader, timeLoc, &time, SHADER_UNIFORM_FLOAT);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(BLACK); // Use a black background to see the glow

            // Use the shader to draw on the whole screen
            BeginShaderMode(shader);
                DrawRectangle(0, 0, screenWidth, screenHeight, WHITE);
            EndShaderMode();

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadShader(shader);
    CloseWindow();
    //--------------------------------------------------------------------------------------

    return 0;
}
