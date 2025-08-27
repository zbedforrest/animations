#include "view_analyzer.h"
#include "raylib.h"
#include <stdio.h>

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
void UpdateAnalyzerView(AppState *state)
{
    Vector2 mousePosition = GetMousePosition();

    // Mouse vs Keyboard control
    if (mousePosition.x != state->lastMousePosition.x || mousePosition.y != state->lastMousePosition.y) {
        state->manualControl = false;
        state->upKeyDownTime = 0.0;
        state->downKeyDownTime = 0.0;
    }
    state->lastMousePosition = mousePosition;

    // Key holds for accelerated movement
    if (IsKeyPressed(KEY_UP)) { state->manualControl = true; state->barY -= 1; state->upKeyDownTime = GetTime(); }
    if (IsKeyDown(KEY_UP) && state->upKeyDownTime > 0.0) { if ((GetTime() - state->upKeyDownTime) > 0.5) state->barY -= 100.0f * GetFrameTime(); }
    if (IsKeyReleased(KEY_UP)) state->upKeyDownTime = 0.0;

    if (IsKeyPressed(KEY_DOWN)) { state->manualControl = true; state->barY += 1; state->downKeyDownTime = GetTime(); }
    if (IsKeyDown(KEY_DOWN) && state->downKeyDownTime > 0.0) { if ((GetTime() - state->downKeyDownTime) > 0.5) state->barY += 100.0f * GetFrameTime(); }
    if (IsKeyReleased(KEY_DOWN)) state->downKeyDownTime = 0.0;

    if (!state->manualControl) state->barY = mousePosition.y;

    // Clamp bar position
    if (state->barY < state->posY) state->barY = state->posY;
    if (state->barY > state->posY + state->finalHeight - 1) state->barY = state->posY + state->finalHeight - 1;

    // Button clicks
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(mousePosition, state->rButton)) state->currentChannel = CHANNEL_R;
        else if (CheckCollisionPointRec(mousePosition, state->gButton)) state->currentChannel = CHANNEL_G;
        else if (CheckCollisionPointRec(mousePosition, state->bButton)) state->currentChannel = CHANNEL_B;
        else if (CheckCollisionPointRec(mousePosition, state->originalButton)) state->currentChannel = CHANNEL_ORIGINAL;
    }

    // Keyframe capture
    if (IsKeyPressed(KEY_K)) {
        float yScale = (float)state->original.height / (float)state->finalHeight;
        int sourceY = (int)((state->barY - state->posY) * yScale);
        if (sourceY >= 0 && sourceY < state->original.height) {
            Color *sourceData = (Color *)state->original.data;
            for (int x = 0; x < state->original.width; x++) {
                state->keyframe_pixels[x] = sourceData[sourceY * state->original.width + x];
            }

            // Regenerate the recreation image with the new keyframe
            Color *newPixels = (Color *)state->recreationImage.data;
            for (int y = 0; y < state->recreationImage.height; y++) {
                bool reversed = ((y / state->stripe_height) % 2 != 0);
                for (int x = 0; x < state->recreationImage.width; x++) {
                    int sourceX = reversed ? (state->recreationImage.width - 1 - x) : x;
                    newPixels[y * state->recreationImage.width + x] = state->keyframe_pixels[sourceX];
                }
            }
            
            if (state->recreationTexture.id > 0) UnloadTexture(state->recreationTexture);
            state->recreationTexture = LoadTextureFromImage(state->recreationImage);
            state->currentView = VIEW_RECREATE;
            SetWindowSize(1000, 800);
        }
    }
}

void DrawAnalyzerView(AppState *state)
{
    Vector2 mousePosition = GetMousePosition();
    Rectangle imageBounds = { (float)state->posX, (float)state->posY, (float)state->finalWidth, (float)state->finalHeight };

    BeginDrawing();
        ClearBackground(BLACK);

        // Draw active texture
        switch (state->currentChannel) {
            case CHANNEL_R: DrawTexture(state->tex_r, state->posX, state->posY, WHITE); break;
            case CHANNEL_G: DrawTexture(state->tex_g, state->posX, state->posY, WHITE); break;
            case CHANNEL_B: DrawTexture(state->tex_b, state->posX, state->posY, WHITE); break;
            case CHANNEL_ORIGINAL: DrawTexture(state->tex_original, state->posX, state->posY, WHITE); break;
        }

        // Draw Plot Area
        DrawRectangleLinesEx(state->plotArea, 1, WHITE);
        DrawText("X-Coordinate", state->plotArea.x + state->plotArea.width/2 - 50, state->plotArea.y + state->plotArea.height + 10, 20, WHITE);
        DrawText("Value", state->plotArea.x - 60, state->plotArea.y + state->plotArea.height/2 - 10, 20, WHITE);

        // Draw hover bar and plot
        if (CheckCollisionPointRec(mousePosition, imageBounds) || state->manualControl) {
            DrawRectangle(state->posX, (int)state->barY - 1, state->finalWidth, 1, GREEN);

            float yScale = (float)state->original.height / (float)state->finalHeight;
            int sourceY = (int)((state->barY - state->posY) * yScale);

            if (sourceY >= 0 && sourceY < state->original.height) {
                if (state->currentChannel == CHANNEL_ORIGINAL) {
                    Vector2 prevR = {0}, prevG = {0}, prevB = {0};
                    Color *sourceData = (Color *)state->original.data;
                    for (int x = 0; x < state->original.width; x++) {
                        Color p = sourceData[sourceY * state->original.width + x];
                        float plotX = state->plotArea.x + ((float)x / (state->original.width - 1)) * state->plotArea.width;
                        Vector2 curR = { plotX, state->plotArea.y + state->plotArea.height - (((float)p.r / 255.0f) * state->plotArea.height) };
                        Vector2 curG = { plotX, state->plotArea.y + state->plotArea.height - (((float)p.g / 255.0f) * state->plotArea.height) };
                        Vector2 curB = { plotX, state->plotArea.y + state->plotArea.height - (((float)p.b / 255.0f) * state->plotArea.height) };
                        if (x > 0) { DrawLineV(prevR, curR, RED); DrawLineV(prevG, curG, GREEN); DrawLineV(prevB, curB, BLUE); }
                        prevR = curR; prevG = curG; prevB = curB;
                    }
                } else {
                    Vector2 prevPoint = { 0 };
                    Color plotColor = RED;
                    Color *sourceData = NULL;
                    switch (state->currentChannel) {
                        case CHANNEL_R: sourceData = (Color *)state->r_img.data; plotColor = RED; break;
                        case CHANNEL_G: sourceData = (Color *)state->g_img.data; plotColor = GREEN; break;
                        case CHANNEL_B: sourceData = (Color *)state->b_img.data; plotColor = BLUE; break;
                        default: break;
                    }
                    for (int x = 0; x < state->original.width; x++) {
                        unsigned char value = 0;
                        switch (state->currentChannel) {
                            case CHANNEL_R: value = sourceData[sourceY * state->original.width + x].r; break;
                            case CHANNEL_G: value = sourceData[sourceY * state->original.width + x].g; break;
                            case CHANNEL_B: value = sourceData[sourceY * state->original.width + x].b; break;
                            default: break;
                        }
                        Vector2 currentPoint = { state->plotArea.x + ((float)x / (state->original.width - 1)) * state->plotArea.width, state->plotArea.y + state->plotArea.height - (((float)value / 255.0f) * state->plotArea.height) };
                        if (x > 0) DrawLineV(prevPoint, currentPoint, plotColor);
                        prevPoint = currentPoint;
                    }
                }
            }
        }

        // Draw Buttons and UI Text
        DrawRectangleRec(state->rButton, (state->currentChannel == CHANNEL_R) ? MAROON : DARKGRAY); DrawText("R", state->rButton.x + 15, state->rButton.y + 5, 20, WHITE);
        DrawRectangleRec(state->gButton, (state->currentChannel == CHANNEL_G) ? DARKGREEN : DARKGRAY); DrawText("G", state->gButton.x + 15, state->gButton.y + 5, 20, WHITE);
        DrawRectangleRec(state->bButton, (state->currentChannel == CHANNEL_B) ? DARKBLUE : DARKGRAY); DrawText("B", state->bButton.x + 15, state->bButton.y + 5, 20, WHITE);
        DrawRectangleRec(state->originalButton, (state->currentChannel == CHANNEL_ORIGINAL) ? PURPLE : DARKGRAY); DrawText("Orig", state->originalButton.x + 15, state->originalButton.y + 5, 20, WHITE);
        DrawText("Press [K] to Keyframe", 260, 15, 20, WHITE);
        DrawFPS(1300, 10);
    EndDrawing();
}
