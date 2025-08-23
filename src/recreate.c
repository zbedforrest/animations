#include "raylib.h"
#include <stdlib.h> // For malloc/free
#include <stdio.h>  // For printf

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum {
    CHANNEL_R,
    CHANNEL_G,
    CHANNEL_B,
    CHANNEL_ORIGINAL
} ActiveChannel;

typedef enum {
    VIEW_ANALYZER,
    VIEW_RECREATE
} AppView;

typedef struct {
    // Core Data
    Image original;
    Image r_img;
    Image g_img;
    Image b_img;

    // Display Textures
    Texture2D tex_r;
    Texture2D tex_g;
    Texture2D tex_b;
    Texture2D tex_original;
    Texture2D recreationTexture;

    // UI and Layout
    int finalWidth;
    int finalHeight;
    int posX;
    int posY;
    Rectangle plotArea;
    Rectangle rButton;
    Rectangle gButton;
    Rectangle bButton;
    Rectangle originalButton;

    // State
    AppView currentView;
    ActiveChannel currentChannel;
    bool manualControl;
    float barY;
    Vector2 lastMousePosition;
    double upKeyDownTime;
    double downKeyDownTime;

    // Keyframe Data
    Color *keyframe_pixels;

} AppState;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
void InitAppState(AppState *state, const char *filename);
void UpdateAnalyzerView(AppState *state);
void DrawAnalyzerView(AppState *state);
void UpdateRecreateView(AppState *state);
void DrawRecreateView(AppState *state);
void Cleanup(AppState *state);

//----------------------------------------------------------------------------------
// Main Entry Point
//----------------------------------------------------------------------------------
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
    InitAppState(&state, argv[1]);

    while (!WindowShouldClose()) {
        switch (state.currentView) {
            case VIEW_ANALYZER:
                UpdateAnalyzerView(&state);
                DrawAnalyzerView(&state);
                break;
            case VIEW_RECREATE:
                UpdateRecreateView(&state);
                DrawRecreateView(&state);
                break;
        }
    }

    Cleanup(&state);
    CloseWindow();

    return 0;
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
void InitAppState(AppState *state, const char *filename)
{
    state->original = LoadImage(filename);
    if (state->original.data == NULL) {
        // In a real app, you might want a more graceful exit
        exit(1);
    }

    // Manual Crop
    Color *pixels = LoadImageColors(state->original);
    int left = state->original.width, right = 0, top = state->original.height, bottom = 0;
    for (int y = 0; y < state->original.height; y++) {
        for (int x = 0; x < state->original.width; x++) {
            Color p = pixels[y * state->original.width + x];
            if (p.r > 10 || p.g > 10 || p.b > 10) {
                if (x < left)   left = x;
                if (x > right)  right = x;
                if (y < top)    top = y;
                if (y > bottom) bottom = y;
            }
        }
    }
    if (left < right && top < bottom) {
        Rectangle cropRec = { (float)left, (float)top, (float)(right - left + 1), (float)(bottom - top + 1) };
        ImageCrop(&state->original, cropRec);
        pixels = LoadImageColors(state->original);
    }

    // Prepare color channels
    Color *r_pixels = (Color *)malloc(state->original.width * state->original.height * sizeof(Color));
    Color *g_pixels = (Color *)malloc(state->original.width * state->original.height * sizeof(Color));
    Color *b_pixels = (Color *)malloc(state->original.width * state->original.height * sizeof(Color));
    for (int i = 0; i < state->original.width * state->original.height; i++) {
        r_pixels[i] = (Color){ pixels[i].r, 0, 0, 255 };
        g_pixels[i] = (Color){ 0, pixels[i].g, 0, 255 };
        b_pixels[i] = (Color){ 0, 0, pixels[i].b, 255 };
    }
    UnloadImageColors(pixels);

    state->r_img = (Image){ r_pixels, state->original.width, state->original.height, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };
    state->g_img = (Image){ g_pixels, state->original.width, state->original.height, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };
    state->b_img = (Image){ b_pixels, state->original.width, state->original.height, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };

    // Calculate layout
    const int screenHeight = 800;
    const int renderAreaWidth = 1000;
    const int renderAreaHeight = 800;
    float aspectRatio = (float)state->original.width / (float)state->original.height;
    int fitWidth, fitHeight;
    if ((renderAreaWidth / aspectRatio) <= renderAreaHeight) {
        fitWidth = renderAreaWidth;
        fitHeight = (int)(fitWidth / aspectRatio);
    } else {
        fitHeight = renderAreaHeight;
        fitWidth = (int)(fitHeight * aspectRatio);
    }
    state->finalWidth = (int)(fitWidth * 0.8f);
    state->finalHeight = (int)(fitHeight * 0.8f);
    state->posX = 0;
    state->posY = (screenHeight - state->finalHeight) / 2;

    // Create display images and textures
    Image r_disp = ImageCopy(state->r_img); ImageResize(&r_disp, state->finalWidth, state->finalHeight); state->tex_r = LoadTextureFromImage(r_disp); UnloadImage(r_disp);
    Image g_disp = ImageCopy(state->g_img); ImageResize(&g_disp, state->finalWidth, state->finalHeight); state->tex_g = LoadTextureFromImage(g_disp); UnloadImage(g_disp);
    Image b_disp = ImageCopy(state->b_img); ImageResize(&b_disp, state->finalWidth, state->finalHeight); state->tex_b = LoadTextureFromImage(b_disp); UnloadImage(b_disp);
    Image o_disp = ImageCopy(state->original); ImageResize(&o_disp, state->finalWidth, state->finalHeight); state->tex_original = LoadTextureFromImage(o_disp); UnloadImage(o_disp);

    // Init UI and State
    state->plotArea = (Rectangle){ (float)state->finalWidth + 50, (float)state->posY, (float)1400 - state->finalWidth - 50, (float)state->finalHeight };
    state->rButton = (Rectangle){ 10, 10, 40, 30 };
    state->gButton = (Rectangle){ 60, 10, 40, 30 };
    state->bButton = (Rectangle){ 110, 10, 40, 30 };
    state->originalButton = (Rectangle){ 160, 10, 80, 30 };
    state->currentChannel = CHANNEL_R;
    state->currentView = VIEW_ANALYZER;
    state->lastMousePosition = (Vector2){ -1.0f, -1.0f };
    state->keyframe_pixels = (Color *)malloc(state->original.width * sizeof(Color));
}

void UpdateAnalyzerView(AppState *state)
{
    Vector2 mousePosition = GetMousePosition();
    Rectangle imageBounds = { (float)state->posX, (float)state->posY, (float)state->finalWidth, (float)state->finalHeight };

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

            Image newImage = GenImageColor(state->original.width, state->original.height, BLACK);
            Color *newPixels = (Color *)newImage.data;
            for (int y = 0; y < newImage.height; y++) {
                bool reversed = ((y / 100) % 2 != 0);
                for (int x = 0; x < newImage.width; x++) {
                    int sourceX = reversed ? (newImage.width - 1 - x) : x;
                    newPixels[y * newImage.width + x] = state->keyframe_pixels[sourceX];
                }
            }
            
            if (state->recreationTexture.id > 0) UnloadTexture(state->recreationTexture);
            state->recreationTexture = LoadTextureFromImage(newImage);
            UnloadImage(newImage);
            state->currentView = VIEW_RECREATE;
        }
    }
}

void DrawAnalyzerView(AppState *state)
{
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
    EndDrawing();
}

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

void Cleanup(AppState *state)
{
    if (state->recreationTexture.id > 0) UnloadTexture(state->recreationTexture);
    free(state->keyframe_pixels);
    UnloadTexture(state->tex_r);
    UnloadTexture(state->tex_g);
    UnloadTexture(state->tex_b);
    UnloadTexture(state->tex_original);
    UnloadImage(state->original);
    UnloadImage(state->r_img);
    UnloadImage(state->g_img);
    UnloadImage(state->b_img);
}