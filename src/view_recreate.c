#include "view_recreate.h"
#include "raylib.h"

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
void UpdateRecreateView(AppState *state)
{
    if (IsKeyPressed(KEY_A)) {
        state->currentView = VIEW_ANALYZER;
    }
}

void DrawRecreateView(AppState *state)
{
    BeginDrawing();
        ClearBackground(BLACK);
        if (state->recreationTexture.id > 0) {
            DrawTexturePro(state->recreationTexture, (Rectangle){ 0, 0, (float)state->recreationTexture.width, (float)state->recreationTexture.height },
                           (Rectangle){ 0, 0, 1400, 800 }, (Vector2){ 0, 0 }, 0.0f, WHITE);
        }
        DrawText("RECREATION VIEW", 10, 10, 20, WHITE);
        DrawText("Press [A] to return to Analyzer", 10, 40, 20, WHITE);
    EndDrawing();
}
