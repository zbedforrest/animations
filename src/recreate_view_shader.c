#include "recreate_view_shader.h"
#include "raylib.h"

void InitRecreateViewShader(AppState *state)
{
    state->recreateShader = LoadShader("src/recreate_view_shader.vs", "src/recreate_view_shader.fs");
}

void UpdateRecreateViewShader(AppState *state)
{
    if (IsKeyPressed(KEY_A)) {
        state->currentView = VIEW_ANALYZER;
        SetWindowSize(1400, 800);
    }
}

void DrawRecreateViewShader(AppState *state)
{
    BeginDrawing();
    ClearBackground(BLACK);

    BeginShaderMode(state->recreateShader);
    DrawTexture(state->tex_original, 0, 0, WHITE);
    EndShaderMode();

    DrawText("RECREATE VIEW SHADER", 10, 10, 20, WHITE);
    DrawFPS(900, 10);
    EndDrawing();
}