#include "raylib.h"
#include <stdlib.h> // For malloc/free
#include <stdio.h>  // For printf

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

    for (int y = 0; y < original.height; y++)
    {
        for (int x = 0; x < original.width; x++)
        {
            Color p = pixels[y * original.width + x];
            if (p.r > 10 || p.g > 10 || p.b > 10)
            {
                if (x < left)   left = x;
                if (x > right)  right = x;
                if (y < top)    top = y;
                if (y > bottom) bottom = y;
            }
        }
    }
    UnloadImageColors(pixels);

    if (left < right && top < bottom)
    {
        Rectangle cropRec = { (float)left, (float)top, (float)(right - left + 1), (float)(bottom - top + 1) };
        ImageCrop(&original, cropRec);
    }
    // --- End Manual Crop ---

    pixels = LoadImageColors(original);
    Color *r_pixels = (Color *)malloc(original.width * original.height * sizeof(Color));
    for (int i = 0; i < original.width * original.height; i++)
    {
        r_pixels[i] = (Color){ pixels[i].r, 0, 0, 255 };
    }
    UnloadImageColors(pixels);

    Image r_img = { .data = r_pixels, .width = original.width, .height = original.height, .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, .mipmaps = 1 };
    Image r_img_display = ImageCopy(r_img);

    // --- Setup Window and Display ---
    const int screenWidth = 1400;
    const int screenHeight = 800;
    InitWindow(screenWidth, screenHeight, "Recreate - Red Channel Analyzer");

    const int renderAreaWidth = 1000;
    const int renderAreaHeight = 800;
    float aspectRatio = (float)r_img_display.width / (float)r_img_display.height;
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
    Texture2D texture = LoadTextureFromImage(r_img_display);

    int posX = 0;
    int posY = (screenHeight - finalHeight) / 2;

    Rectangle plotArea = { (float)finalWidth + 50, (float)posY, (float)screenWidth - finalWidth - 100, (float)finalHeight };

    SetTargetFPS(60);

    // Main game loop
    while (!WindowShouldClose())
    {
        Vector2 mousePosition = GetMousePosition();
        Rectangle imageBounds = { (float)posX, (float)posY, (float)finalWidth, (float)finalHeight };

        BeginDrawing();
            ClearBackground(BLACK);
            DrawTexture(texture, posX, posY, WHITE);

            // Draw Plot Area
            DrawRectangleLinesEx(plotArea, 1, WHITE);
            DrawText("X-Coordinate", plotArea.x + plotArea.width/2 - 50, plotArea.y + plotArea.height + 10, 20, WHITE);
            DrawText("R-Value", plotArea.x - 80, plotArea.y + plotArea.height/2 - 10, 20, WHITE);


            if (CheckCollisionPointRec(mousePosition, imageBounds))
            {
                // Draw the horizontal green bar (1px thick)
                DrawRectangle(posX, (int)mousePosition.y - 1, finalWidth, 1, GREEN);

                // --- Plotting Logic ---
                float yScale = (float)r_img.height / (float)finalHeight;
                int sourceY = (int)((mousePosition.y - posY) * yScale);

                if (sourceY >= 0 && sourceY < r_img.height)
                {
                    Vector2 prevPoint = { 0 };
                    for (int x = 0; x < r_img.width; x++)
                    {
                        unsigned char redValue = ((Color *)r_img.data)[sourceY * r_img.width + x].r;
                        
                        Vector2 currentPoint = {
                            plotArea.x + ((float)x / (r_img.width - 1)) * plotArea.width,
                            plotArea.y + plotArea.height - (((float)redValue / 255.0f) * plotArea.height)
                        };

                        if (x > 0) DrawLineV(prevPoint, currentPoint, RED);
                        prevPoint = currentPoint;
                    }
                }
            }

        EndDrawing();
    }

    // --- Cleanup ---
    UnloadTexture(texture);
    CloseWindow();

    UnloadImage(original);
    UnloadImage(r_img);
    UnloadImage(r_img_display);

    return 0;
}
