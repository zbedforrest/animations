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

    // Crop transparent pixels from the edges
    ImageAlphaCrop(&original, 0.0f);

    // Get pixel data for the entire image
    Color *pixels = LoadImageColors(original);
    Color *r_pixels = (Color *)malloc(original.width * original.height * sizeof(Color));

    // Create the red channel pixel data
    for (int i = 0; i < original.width * original.height; i++)
    {
        r_pixels[i] = (Color){ pixels[i].r, 0, 0, 255 };
    }

    // Create an image from the red channel pixel data
    Image r_img =
    {
        .data = r_pixels,
        .width = original.width,
        .height = original.height,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        .mipmaps = 1
    };

    // --- Setup Window and Display ---

    const int screenWidth = 1400;
    const int screenHeight = 800;
    InitWindow(screenWidth, screenHeight, "Recreate - Red Channel");

    // Define the original render area to calculate the base size
    const int renderAreaWidth = 1000;
    const int renderAreaHeight = 800;

    // Calculate the size to fit the original render area
    float aspectRatio = (float)r_img.width / (float)r_img.height;
    int fitWidth, fitHeight;
    if ((renderAreaWidth / aspectRatio) <= renderAreaHeight)
    {
        fitWidth = renderAreaWidth;
        fitHeight = (int)(fitWidth / aspectRatio);
    }
    else
    {
        fitHeight = renderAreaHeight;
        fitWidth = (int)(fitHeight * aspectRatio);
    }

    // Calculate the final size (80% of the fit size)
    int finalWidth = (int)(fitWidth * 0.8f);
    int finalHeight = (int)(fitHeight * 0.8f);

    // Resize the red image to the final size
    ImageResize(&r_img, finalWidth, finalHeight);

    // Create a texture from the resized image
    Texture2D texture = LoadTextureFromImage(r_img);

    // Position the texture on the left side, centered vertically
    int posX = 0; // Flush against the left edge
    int posY = (screenHeight - finalHeight) / 2;

    SetTargetFPS(60);

    // Main game loop
    while (!WindowShouldClose())
    {
        Vector2 mousePosition = GetMousePosition();
        Rectangle imageBounds = { (float)posX, (float)posY, (float)finalWidth, (float)finalHeight };

        BeginDrawing();
            ClearBackground(BLACK);
            DrawTexture(texture, posX, posY, WHITE);

            // Check if mouse is over the image
            if (CheckCollisionPointRec(mousePosition, imageBounds))
            {
                // Draw the horizontal green bar
                DrawRectangle(posX, (int)mousePosition.y - 2, finalWidth, 4, GREEN);
            }

        EndDrawing();
    }

    // --- Cleanup ---
    UnloadTexture(texture);
    CloseWindow();

    UnloadImage(original);
    UnloadImageColors(pixels);
    UnloadImage(r_img);

    return 0;
}
