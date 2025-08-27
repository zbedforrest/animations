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
    Vector2 mousePosition = GetMousePosition();

    // Define Animate button here as its position is calculated dynamically
    state->animateButton = (Rectangle){ 10, 100, 120, 30 };

    // Return to analyzer
    if (IsKeyPressed(KEY_A)) {
        state->currentView = VIEW_ANALYZER;
        state->animating = false; // Stop animation when leaving view
        SetWindowSize(1400, 800);
    }

    // Toggle animation
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePosition, state->animateButton)) {
        state->animating = !state->animating;
    }

    // Update t if animating
    if (state->animating) {
        state->t += state->dt;
        RegenerateRecreationImage(state); // Regenerate on each frame for animation
        UpdateTexture(state->recreationTexture, state->recreationImage.data);
    }

    // Handle LEFT key
    if (IsKeyPressed(KEY_LEFT)) {
        state->z_offset--;
        z_changed = true;
        state->leftKeyDownTime = GetTime();
    }
    if (IsKeyDown(KEY_LEFT) && state->leftKeyDownTime > 0.0) {
        if ((GetTime() - state->leftKeyDownTime) > 0.5) {
            state->z_offset -= (int)(100.0f * GetFrameTime());
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
            state->z_offset += (int)(100.0f * GetFrameTime());
            z_changed = true;
        }
    }
    if (IsKeyReleased(KEY_RIGHT)) {
        state->rightKeyDownTime = 0.0;
    }

    // If Z changed, regenerate the image data and update the GPU texture
    if (z_changed && !state->animating) { // Avoid double-update when animating
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

        // Draw Animate Button
        DrawRectangleRec(state->animateButton, state->animating ? DARKGREEN : DARKGRAY);
        DrawText(state->animating ? "Animating..." : "Animate", state->animateButton.x + 10, state->animateButton.y + 5, 20, WHITE);


        // Draw Z-Offset and t values on the bottom right
        const int fontSize = 20;
        const int padding = 10;
        const char *zText = TextFormat("Z: %d", state->z_offset);
        const char *tText = TextFormat("t: %.3f", state->t);
        int zTextWidth = MeasureText(zText, fontSize);
        int tTextWidth = MeasureText(tText, fontSize);

        DrawText(zText, 1000 - zTextWidth - padding, 800 - (fontSize * 2) - (padding * 2), fontSize, GREEN);
        DrawText(tText, 1000 - tTextWidth - padding, 800 - fontSize - padding, fontSize, GREEN);
        DrawFPS(900, 10);
    EndDrawing();
}

//----------------------------------------------------------------------------------
// Module Local Functions Definition
//----------------------------------------------------------------------------------
static unsigned char get_animated_r(AppState *state, int x, bool reversed) {
    int width = state->recreationImage.width;
    int sourceX_r;
    if (reversed) {
        sourceX_r = ((x + state->z_offset - (int)state->t) % width + width) % width;
    } else {
        sourceX_r = ((x + state->z_offset + (int)state->t) % width + width) % width;
    }
    return state->keyframe_pixels[sourceX_r].r;
}

static unsigned char get_animated_g(AppState *state, int x, bool reversed) {
    int width = state->recreationImage.width;
    int sourceX_g;
    if (reversed) {
        sourceX_g = ((x + state->z_offset - (int)state->t) % width + width) % width;
    } else {
        sourceX_g = ((x + state->z_offset + (int)state->t) % width + width) % width;
    }
    return state->keyframe_pixels[sourceX_g].g;
}

static unsigned char get_animated_b(AppState *state, int x, bool reversed) {
    int width = state->recreationImage.width;
    int sourceX_b;
    if (reversed) {
        sourceX_b = ((x + state->z_offset - (int)state->t) % width + width) % width;
    } else {
        sourceX_b = ((x + state->z_offset + (int)state->t) % width + width) % width;
    }
    return state->keyframe_pixels[sourceX_b].b;
}

static void RegenerateRecreationImage(AppState *state)
{
    Color *newPixels = (Color *)state->recreationImage.data;
    int width = state->recreationImage.width;
    int height = state->recreationImage.height;

    for (int y = 0; y < height; y++) {
        bool reversed = ((y / state->stripe_height) % 2 != 0);
        for (int x = 0; x < width; x++) {
            Color *pixel = &newPixels[y * width + x];
            if (state->animating) {
                pixel->r = get_animated_r(state, x, reversed);
                pixel->g = get_animated_g(state, x, reversed);
                pixel->b = get_animated_b(state, x, reversed);
                pixel->a = 255;
            } else {
                int sourceX;
                if (reversed) {
                    sourceX = ( ( (width - 1 - x) + state->z_offset) % width + width ) % width;
                } else {
                    sourceX = x;
                }
                *pixel = state->keyframe_pixels[sourceX];
            }
        }
    }
}