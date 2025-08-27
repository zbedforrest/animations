#include "app.h"
#include "view_analyzer.h"
#include "view_recreate.h"
#include "recreate_view_shader.h"
#include <stdlib.h> // For exit()

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
void InitApp(AppState *state, const char *filename)
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
    state->shadermodeButton = (Rectangle){ 250, 10, 80, 30 };
    state->currentChannel = CHANNEL_R;
    state->currentView = VIEW_ANALYZER;
    state->lastMousePosition = (Vector2){ -1.0f, -1.0f };
    state->keyframe_pixels = (Color *)malloc(state->original.width * sizeof(Color));
    state->recreationImage = GenImageColor(state->original.width, state->original.height, BLACK);
    state->z_offset = 0;
    state->stripe_height = 400; // Default stripe height
    state->animating = true;
    state->t = 0.0f;
    state->dt = 1.0f;

    InitRecreateViewShader(state);
}

void RunApp(AppState *state)
{
    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_S)) {
            state->currentView = VIEW_RECREATE_SHADER;
        }

        switch (state->currentView) {
            case VIEW_ANALYZER:
                UpdateAnalyzerView(state);
                DrawAnalyzerView(state);
                break;
            case VIEW_RECREATE:
                UpdateRecreateView(state);
                DrawRecreateView(state);
                break;
            case VIEW_RECREATE_SHADER:
                UpdateRecreateViewShader(state);
                DrawRecreateViewShader(state);
                break;
        }
    }
}

void CleanupApp(AppState *state)
{
    if (state->recreationTexture.id > 0) UnloadTexture(state->recreationTexture);
    free(state->keyframe_pixels);
    UnloadTexture(state->tex_r);
    UnloadTexture(state->tex_g);
    UnloadTexture(state->tex_b);
    UnloadTexture(state->tex_original);
    UnloadImage(state->original);
    free(state->r_img.data);
    free(state->g_img.data);
    free(state->b_img.data);
    UnloadImage(state->recreationImage);
}
