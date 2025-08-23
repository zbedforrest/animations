#include "raylib.h"
#include <stdlib.h> // For malloc/free
#include <stdio.h>  // For printf

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

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <image_file>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    printf("Processing %s...\n", filename);

    // Load the original image
    Image original = LoadImage(filename);
    if (original.data == NULL)
    {
        printf("Failed to load image: %s\n", filename);
        return 1;
    }

    // --- Manual Crop Logic ---
    Color *pixels = LoadImageColors(original);
    int left = original.width, right = 0, top = original.height, bottom = 0;
    for (int y = 0; y < original.height; y++) {
        for (int x = 0; x < original.width; x++) {
            Color p = pixels[y * original.width + x];
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
        ImageCrop(&original, cropRec);
        pixels = LoadImageColors(original); // Reload pixels from cropped image
    }
    // --- End Manual Crop ---

    // --- Prepare all three color channels ---
    Color *r_pixels = (Color *)malloc(original.width * original.height * sizeof(Color));
    Color *g_pixels = (Color *)malloc(original.width * original.height * sizeof(Color));
    Color *b_pixels = (Color *)malloc(original.width * original.height * sizeof(Color));
    for (int i = 0; i < original.width * original.height; i++) {
        r_pixels[i] = (Color){ pixels[i].r, 0, 0, 255 };
        g_pixels[i] = (Color){ 0, pixels[i].g, 0, 255 };
        b_pixels[i] = (Color){ 0, 0, pixels[i].b, 255 };
    }
    UnloadImageColors(pixels);

    Image r_img = { .data = r_pixels, .width = original.width, .height = original.height, .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, .mipmaps = 1 };
    Image g_img = { .data = g_pixels, .width = original.width, .height = original.height, .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, .mipmaps = 1 };
    Image b_img = { .data = b_pixels, .width = original.width, .height = original.height, .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, .mipmaps = 1 };

    Image r_img_display = ImageCopy(r_img);
    Image g_img_display = ImageCopy(g_img);
    Image b_img_display = ImageCopy(b_img);
    Image original_display = ImageCopy(original);

    // --- Setup Window and Display ---
    const int screenWidth = 1400;
    const int screenHeight = 800;
    InitWindow(screenWidth, screenHeight, "Image Tool");

    const int renderAreaWidth = 1000;
    const int renderAreaHeight = 800;
    float aspectRatio = (float)original.width / (float)original.height;
    int fitWidth, fitHeight;
    if ((renderAreaWidth / aspectRatio) <= renderAreaHeight) {
        fitWidth = renderAreaWidth;
        fitHeight = (int)(fitWidth / aspectRatio);
    } else {
        fitHeight = renderAreaHeight;
        fitWidth = (int)(fitHeight * aspectRatio);
    }

    int finalWidth = (int)(fitWidth * 0.8f);
    int finalHeight = (int)(fitHeight * 0.8f);
    ImageResize(&r_img_display, finalWidth, finalHeight);
    ImageResize(&g_img_display, finalWidth, finalHeight);
    ImageResize(&b_img_display, finalWidth, finalHeight);
    ImageResize(&original_display, finalWidth, finalHeight);

    Texture2D tex_r = LoadTextureFromImage(r_img_display);
    Texture2D tex_g = LoadTextureFromImage(g_img_display);
    Texture2D tex_b = LoadTextureFromImage(b_img_display);
    Texture2D tex_original = LoadTextureFromImage(original_display);

    int posX = 0;
    int posY = (screenHeight - finalHeight) / 2;

    Rectangle plotArea = { (float)finalWidth + 50, (float)posY, (float)screenWidth - finalWidth - 50, (float)finalHeight };

    // --- UI Elements & State ---
    Rectangle rButton = { 10, 10, 40, 30 };
    Rectangle gButton = { 60, 10, 40, 30 };
    Rectangle bButton = { 110, 10, 40, 30 };
    Rectangle originalButton = { 160, 10, 80, 30 };
    ActiveChannel currentChannel = CHANNEL_R;
    AppView currentView = VIEW_ANALYZER;

    bool manualControl = false;
    float barY = 0;
    Vector2 lastMousePosition = { -1.0f, -1.0f };

    double upKeyDownTime = 0.0;
    double downKeyDownTime = 0.0;
    const double HOLD_THRESHOLD = 0.5;
    const float MOVE_SPEED = 100.0f;

    // --- Keyframe Data ---
    Color *keyframe_pixels = (Color *)malloc(original.width * sizeof(Color));
    Texture2D recreationTexture = { 0 };

    SetTargetFPS(60);

    // Main game loop
    while (!WindowShouldClose())
    {
        Vector2 mousePosition = GetMousePosition();
        Rectangle imageBounds = { (float)posX, (float)posY, (float)finalWidth, (float)finalHeight };

        // --- Global Input ---
        if (IsKeyPressed(KEY_A)) currentView = VIEW_ANALYZER;

        // --- ANALYZER VIEW ---
        if (currentView == VIEW_ANALYZER)
        {
            // --- Update (Input Handling) ---
            if (mousePosition.x != lastMousePosition.x || mousePosition.y != lastMousePosition.y) {
                manualControl = false;
                upKeyDownTime = 0.0;
                downKeyDownTime = 0.0;
            }
            lastMousePosition = mousePosition;

            if (IsKeyPressed(KEY_UP)) { manualControl = true; barY -= 1; upKeyDownTime = GetTime(); }
            if (IsKeyDown(KEY_UP) && upKeyDownTime > 0.0) { if ((GetTime() - upKeyDownTime) > HOLD_THRESHOLD) barY -= MOVE_SPEED * GetFrameTime(); }
            if (IsKeyReleased(KEY_UP)) upKeyDownTime = 0.0;

            if (IsKeyPressed(KEY_DOWN)) { manualControl = true; barY += 1; downKeyDownTime = GetTime(); }
            if (IsKeyDown(KEY_DOWN) && downKeyDownTime > 0.0) { if ((GetTime() - downKeyDownTime) > HOLD_THRESHOLD) barY += MOVE_SPEED * GetFrameTime(); }
            if (IsKeyReleased(KEY_DOWN)) downKeyDownTime = 0.0;

            if (!manualControl) barY = mousePosition.y;

            if (barY < posY) barY = posY;
            if (barY > posY + finalHeight - 1) barY = posY + finalHeight - 1;

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (CheckCollisionPointRec(mousePosition, rButton)) currentChannel = CHANNEL_R;
                else if (CheckCollisionPointRec(mousePosition, gButton)) currentChannel = CHANNEL_G;
                else if (CheckCollisionPointRec(mousePosition, bButton)) currentChannel = CHANNEL_B;
                else if (CheckCollisionPointRec(mousePosition, originalButton)) currentChannel = CHANNEL_ORIGINAL;
            }

            if (IsKeyPressed(KEY_K)) {
                float yScale = (float)original.height / (float)finalHeight;
                int sourceY = (int)((barY - posY) * yScale);
                if (sourceY >= 0 && sourceY < original.height) {
                    Color *sourceData = (Color *)original.data;
                    for (int x = 0; x < original.width; x++) {
                        keyframe_pixels[x] = sourceData[sourceY * original.width + x];
                    }

                    // Generate the recreation image
                    Image newImage = GenImageColor(original.width, original.height, BLACK);
                    Color *newPixels = (Color *)newImage.data;
                    for (int y = 0; y < newImage.height; y++) {
                        for (int x = 0; x < newImage.width; x++) {
                            newPixels[y * newImage.width + x] = keyframe_pixels[x];
                        }
                    }
                    
                    if (recreationTexture.id > 0) UnloadTexture(recreationTexture);
                    recreationTexture = LoadTextureFromImage(newImage);
                    UnloadImage(newImage);
                    currentView = VIEW_RECREATE;
                }
            }

            // --- Draw ---
            BeginDrawing();
                ClearBackground(BLACK);
                switch (currentChannel) {
                    case CHANNEL_R: DrawTexture(tex_r, posX, posY, WHITE); break;
                    case CHANNEL_G: DrawTexture(tex_g, posX, posY, WHITE); break;
                    case CHANNEL_B: DrawTexture(tex_b, posX, posY, WHITE); break;
                    case CHANNEL_ORIGINAL: DrawTexture(tex_original, posX, posY, WHITE); break;
                }
                DrawRectangleLinesEx(plotArea, 1, WHITE);
                DrawText("X-Coordinate", plotArea.x + plotArea.width/2 - 50, plotArea.y + plotArea.height + 10, 20, WHITE);
                DrawText("Value", plotArea.x - 60, plotArea.y + plotArea.height/2 - 10, 20, WHITE);

                if (CheckCollisionPointRec(mousePosition, imageBounds) || manualControl) {
                    DrawRectangle(posX, (int)barY - 1, finalWidth, 1, GREEN);
                    
                    // --- Plotting Logic ---
                    float yScale = (float)original.height / (float)finalHeight;
                    int sourceY = (int)((barY - posY) * yScale);

                    if (sourceY >= 0 && sourceY < original.height) {
                        if (currentChannel == CHANNEL_ORIGINAL) {
                            Vector2 prevR = {0}, prevG = {0}, prevB = {0};
                            Color *sourceData = (Color *)original.data;
                            for (int x = 0; x < original.width; x++) {
                                Color p = sourceData[sourceY * original.width + x];
                                float plotX = plotArea.x + ((float)x / (original.width - 1)) * plotArea.width;
                                
                                Vector2 curR = { plotX, plotArea.y + plotArea.height - (((float)p.r / 255.0f) * plotArea.height) };
                                Vector2 curG = { plotX, plotArea.y + plotArea.height - (((float)p.g / 255.0f) * plotArea.height) };
                                Vector2 curB = { plotX, plotArea.y + plotArea.height - (((float)p.b / 255.0f) * plotArea.height) };

                                if (x > 0) {
                                    DrawLineV(prevR, curR, RED);
                                    DrawLineV(prevG, curG, GREEN);
                                    DrawLineV(prevB, curB, BLUE);
                                }
                                prevR = curR; prevG = curG; prevB = curB;
                            }
                        } else {
                            Vector2 prevPoint = { 0 };
                            Color plotColor = RED;
                            Color *sourceData = NULL;
                            switch (currentChannel) {
                                case CHANNEL_R: sourceData = (Color *)r_img.data; plotColor = RED; break;
                                case CHANNEL_G: sourceData = (Color *)g_img.data; plotColor = GREEN; break;
                                case CHANNEL_B: sourceData = (Color *)b_img.data; plotColor = BLUE; break;
                                case CHANNEL_ORIGINAL: break; // Should not happen
                            }

                            for (int x = 0; x < original.width; x++) {
                                unsigned char value = 0;
                                switch (currentChannel) {
                                    case CHANNEL_R: value = sourceData[sourceY * original.width + x].r; break;
                                    case CHANNEL_G: value = sourceData[sourceY * original.width + x].g; break;
                                    case CHANNEL_B: value = sourceData[sourceY * original.width + x].b; break;
                                    case CHANNEL_ORIGINAL: break; // Should not happen
                                }
                                
                                Vector2 currentPoint = {
                                    plotArea.x + ((float)x / (original.width - 1)) * plotArea.width,
                                    plotArea.y + plotArea.height - (((float)value / 255.0f) * plotArea.height)
                                };

                                if (x > 0) DrawLineV(prevPoint, currentPoint, plotColor);
                                prevPoint = currentPoint;
                            }
                        }
                    }
                }
                DrawRectangleRec(rButton, (currentChannel == CHANNEL_R) ? MAROON : DARKGRAY);
                DrawText("R", rButton.x + 15, rButton.y + 5, 20, WHITE);
                DrawRectangleRec(gButton, (currentChannel == CHANNEL_G) ? DARKGREEN : DARKGRAY);
                DrawText("G", gButton.x + 15, gButton.y + 5, 20, WHITE);
                DrawRectangleRec(bButton, (currentChannel == CHANNEL_B) ? DARKBLUE : DARKGRAY);
                DrawText("B", bButton.x + 15, bButton.y + 5, 20, WHITE);
                DrawRectangleRec(originalButton, (currentChannel == CHANNEL_ORIGINAL) ? PURPLE : DARKGRAY);
                DrawText("Orig", originalButton.x + 15, originalButton.y + 5, 20, WHITE);
                DrawText("Press [K] to Keyframe", 260, 15, 20, WHITE);
            EndDrawing();
        }
        // --- RECREATE VIEW ---
        else if (currentView == VIEW_RECREATE)
        {
            BeginDrawing();
                ClearBackground(BLACK);
                if (recreationTexture.id > 0) {
                    DrawTexturePro(recreationTexture, (Rectangle){ 0, 0, (float)recreationTexture.width, (float)recreationTexture.height },
                                   (Rectangle){ 0, 0, (float)screenWidth, (float)screenHeight }, (Vector2){ 0, 0 }, 0.0f, WHITE);
                }
                DrawText("RECREATION VIEW", 10, 10, 20, WHITE);
                DrawText("Press [A] to return to Analyzer", 10, 40, 20, WHITE);
            EndDrawing();
        }
    }

    // --- Cleanup ---
    if (recreationTexture.id > 0) UnloadTexture(recreationTexture);
    free(keyframe_pixels);
    UnloadTexture(tex_r);
    UnloadTexture(tex_g);
    UnloadTexture(tex_b);
    UnloadTexture(tex_original);
    CloseWindow();
    UnloadImage(original);
    UnloadImage(r_img);
    UnloadImage(g_img);
    UnloadImage(b_img);
    UnloadImage(r_img_display);
    UnloadImage(g_img_display);
    UnloadImage(b_img_display);
    UnloadImage(original_display);

    return 0;
}