#include "view_recreate.h"
#include "raylib.h"
#include <stdio.h> // For TextFormat

//----------------------------------------------------------------------------------
// Module Local Functions Declaration
//----------------------------------------------------------------------------------
static void RegenerateRecreationImage(AppState *state);

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
void UpdateRecreateView(AppState *state)
{
    bool z_changed = false;

    // Return to analyzer
    if (IsKeyPressed(KEY_A)) {
        state->currentView = VIEW_ANALYZER;
        SetWindowSize(1400, 800);
    }

    // Handle LEFT key
    if (IsKeyPressed(KEY_LEFT)) {
        state->z_offset--;
        z_changed = true;
        state->leftKeyDownTime = GetTime();
    }
    if (IsKeyDown(KEY_LEFT) && state->leftKeyDownTime > 0.0) {
        if ((GetTime() - state->leftKeyDownTime) > 0.5) {
            state->z_offset -= (int)(400.0f * GetFrameTime());
            z_changed = true;
        }
    }
    if (IsKeyReleased(KEY_LEFT)) {
        state->leftKeyDownTime = 0.0;
    }

    // Handle RIGHT key
    if (IsKeyPressed(KEY_RIGHT)) {
        state->z_offset++;
        z_changed = true;
        state->rightKeyDownTime = GetTime();
    }
    if (IsKeyDown(KEY_RIGHT) && state->rightKeyDownTime > 0.0) {
        if ((GetTime() - state->rightKeyDownTime) > 0.5) {
            state->z_offset += (int)(400.0f * GetFrameTime());
            z_changed = true;
        }
    }
    if (IsKeyReleased(KEY_RIGHT)) {
        state->rightKeyDownTime = 0.0;
    }

    // If Z changed, regenerate the image data and update the GPU texture
    if (z_changed) {
        RegenerateRecreationImage(state);
        UpdateTexture(state->recreationTexture, state->recreationImage.data);
    }
}

void DrawRecreateView(AppState *state)
{
    BeginDrawing();
        ClearBackground(BLACK);
        if (state->recreationTexture.id > 0) {
            DrawTexturePro(state->recreationTexture, (Rectangle){ 0, 0, (float)state->recreationTexture.width, (float)state->recreationTexture.height },
                           (Rectangle){ 0, 0, 1000, 800 }, (Vector2){ 0, 0 }, 0.0f, WHITE);
        }
        DrawText("RECREATION VIEW", 10, 10, 20, WHITE);
        DrawText("Press [A] to return to Analyzer", 10, 40, 20, WHITE);
        DrawText("Use Left/Right Arrows to change Z-Offset", 10, 70, 20, WHITE);

        // Draw Z-Offset value on the bottom right
        const int fontSize = 20;
        const int padding = 10;
        const char *zText = TextFormat("Z: %d", state->z_offset);
        int textWidth = MeasureText(zText, fontSize);
        DrawText(zText, 1000 - textWidth - padding, 800 - fontSize - padding, fontSize, GREEN);
    EndDrawing();
}

//----------------------------------------------------------------------------------
// Module Local Functions Definition
//----------------------------------------------------------------------------------
static void RegenerateRecreationImage(AppState *state)
{
    Color *newPixels = (Color *)state->recreationImage.data;
    int width = state->recreationImage.width;
    int height = state->recreationImage.height;

    for (int y = 0; y < height; y++) {
        bool reversed = ((y / state->stripe_height) % 2 != 0);
        for (int x = 0; x < width; x++) {
            int sourceX;
            if (reversed) {
                // Positive modulo for negative numbers
                sourceX = ( ( (width - 1 - x) + state->z_offset) % width + width ) % width;
            } else {
                sourceX = x;
            }
            newPixels[y * width + x] = state->keyframe_pixels[sourceX];
        }
    }
}
