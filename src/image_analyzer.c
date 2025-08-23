#include "raylib.h"
#include <stdlib.h> // For malloc/free

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    Image target = LoadImage("TARGET.png");
    float aspectRatio = (float)target.width / (float)target.height;

    // Define thumbnail width and calculate height
    int thumbWidth = 350;
    int thumbHeight = (int)(thumbWidth / aspectRatio);

    // Define display image width and calculate height
    int displayWidth = 700;
    int displayHeight = (int)(displayWidth / aspectRatio);

    // Define spacing
    int spacing = 20;

    // Calculate screen dimensions
    const int screenWidth = (thumbWidth * 3) + (spacing * 4);
    const int screenHeight = displayHeight + thumbHeight + (spacing * 3);

    InitWindow(screenWidth, screenHeight, "Image Color Bands");

    // Create a copy for thumbnails and resize it
    Image thumb = ImageCopy(target);
    ImageResize(&thumb, thumbWidth, thumbHeight);

    // Resize the main display image
    ImageResize(&target, displayWidth, displayHeight);

    // Get pixel data from the thumbnail image
    Color *pixels = LoadImageColors(thumb);

    // Create pixel data for color bands
    Color *r_pixels = (Color *)malloc(thumb.width * thumb.height * sizeof(Color));
    Color *g_pixels = (Color *)malloc(thumb.width * thumb.height * sizeof(Color));
    Color *b_pixels = (Color *)malloc(thumb.width * thumb.height * sizeof(Color));

    // Separate color bands
    for (int i = 0; i < thumb.width * thumb.height; i++)
    {
        r_pixels[i] = (Color){ pixels[i].r, 0, 0, 255 };
        g_pixels[i] = (Color){ 0, pixels[i].g, 0, 255 };
        b_pixels[i] = (Color){ 0, 0, pixels[i].b, 255 };
    }

    // Create images from pixel data
    Image r_img = { .data = r_pixels, .width = thumb.width, .height = thumb.height, .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, .mipmaps = 1 };
    Image g_img = { .data = g_pixels, .width = thumb.width, .height = thumb.height, .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, .mipmaps = 1 };
    Image b_img = { .data = b_pixels, .width = thumb.width, .height = thumb.height, .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, .mipmaps = 1 };

    // Create textures
    Texture2D tex_original = LoadTextureFromImage(target);
    Texture2D tex_r = LoadTextureFromImage(r_img);
    Texture2D tex_g = LoadTextureFromImage(g_img);
    Texture2D tex_b = LoadTextureFromImage(b_img);

    // Unload image data
    UnloadImageColors(pixels);
    UnloadImage(target);
    UnloadImage(thumb);

    // Free the memory allocated for the color channels
    free(r_pixels);
    free(g_pixels);
    free(b_pixels);

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())
    {
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            // Draw original image centered on the top row
            int original_x = (screenWidth / 2) - (tex_original.width / 2);
            int original_y = spacing;
            DrawText("Original", original_x, original_y - 15, 20, BLACK);
            DrawTexture(tex_original, original_x, original_y, WHITE);

            // Draw RGB channels on the bottom row
            int thumb_y = original_y + tex_original.height + spacing;
            int r_x = spacing;
            int g_x = spacing * 2 + thumbWidth;
            int b_x = spacing * 3 + thumbWidth * 2;

            DrawText("Red Channel", r_x, thumb_y - 15, 20, RED);
            DrawTexture(tex_r, r_x, thumb_y, WHITE);

            DrawText("Green Channel", g_x, thumb_y - 15, 20, GREEN);
            DrawTexture(tex_g, g_x, thumb_y, WHITE);

            DrawText("Blue Channel", b_x, thumb_y - 15, 20, BLUE);
            DrawTexture(tex_b, b_x, thumb_y, WHITE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(tex_original);
    UnloadTexture(tex_r);
    UnloadTexture(tex_g);
    UnloadTexture(tex_b);

    CloseWindow();
    //--------------------------------------------------------------------------------------

    return 0;
}
