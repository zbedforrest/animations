#include "raylib.h"
#include <stdlib.h> // For malloc/free
#include <stdio.h>  // For printf, sprintf

void process_image(const char *filename);

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s [image files...]\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++)
    {
        process_image(argv[i]);
    }

    return 0;
}

void process_image(const char *filename)
{
    printf("Processing %s...\n", filename);

    Image original = LoadImage(filename);
    if (original.data == NULL)
    {
        printf("Failed to load image: %s\n", filename);
        return;
    }

    float aspectRatio = (float)original.width / (float)original.height;

    // Define image sizes and spacing
    int thumbWidth = 350;
    int thumbHeight = (int)(thumbWidth / aspectRatio);
    int displayWidth = 700;
    int displayHeight = (int)(displayWidth / aspectRatio);
    int spacing = 20;

    // Calculate final canvas dimensions
    int canvasWidth = (thumbWidth * 3) + (spacing * 4);
    int canvasHeight = displayHeight + thumbHeight + (spacing * 3);

    // Create the canvas to draw on
    Image canvas = GenImageColor(canvasWidth, canvasHeight, RAYWHITE);

    // Create and resize a copy for the large display image
    Image displayImg = ImageCopy(original);
    ImageResize(&displayImg, displayWidth, displayHeight);

    // Create and resize a copy for the thumbnails
    Image thumbImg = ImageCopy(original);
    ImageResize(&thumbImg, thumbWidth, thumbHeight);

    // Get pixel data from the thumbnail image for channel separation
    Color *pixels = LoadImageColors(thumbImg);
    Color *r_pixels = (Color *)malloc(thumbImg.width * thumbImg.height * sizeof(Color));
    Color *g_pixels = (Color *)malloc(thumbImg.width * thumbImg.height * sizeof(Color));
    Color *b_pixels = (Color *)malloc(thumbImg.width * thumbImg.height * sizeof(Color));

    for (int i = 0; i < thumbImg.width * thumbImg.height; i++)
    {
        r_pixels[i] = (Color){ pixels[i].r, 0, 0, 255 };
        g_pixels[i] = (Color){ 0, pixels[i].g, 0, 255 };
        b_pixels[i] = (Color){ 0, 0, pixels[i].b, 255 };
    }

    Image r_img = { .data = r_pixels, .width = thumbWidth, .height = thumbHeight, .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, .mipmaps = 1 };
    Image g_img = { .data = g_pixels, .width = thumbWidth, .height = thumbHeight, .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, .mipmaps = 1 };
    Image b_img = { .data = b_pixels, .width = thumbWidth, .height = thumbHeight, .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, .mipmaps = 1 };

    // --- Draw the layout onto the canvas ---

    // Draw original image (centered top)
    int displayX = (canvasWidth / 2) - (displayWidth / 2);
    ImageDraw(&canvas, displayImg, (Rectangle){0, 0, displayWidth, displayHeight}, (Rectangle){displayX, spacing, displayWidth, displayHeight}, WHITE);

    // Draw RGB channels (bottom row)
    int thumbY = displayHeight + (spacing * 2);
    int r_x = spacing;
    int g_x = spacing * 2 + thumbWidth;
    int b_x = spacing * 3 + thumbWidth * 2;

    ImageDraw(&canvas, r_img, (Rectangle){0, 0, thumbWidth, thumbHeight}, (Rectangle){r_x, thumbY, thumbWidth, thumbHeight}, WHITE);
    ImageDraw(&canvas, g_img, (Rectangle){0, 0, thumbWidth, thumbHeight}, (Rectangle){g_x, thumbY, thumbWidth, thumbHeight}, WHITE);
    ImageDraw(&canvas, b_img, (Rectangle){0, 0, thumbWidth, thumbHeight}, (Rectangle){b_x, thumbY, thumbWidth, thumbHeight}, WHITE);

    // Export the final composite image
    char out_filename[100];
    sprintf(out_filename, "analysis_output/%s_analysis.png", filename);
    ExportImage(canvas, out_filename);

    printf("Saved composite analysis to %s\n", out_filename);

    // --- Cleanup ---
    UnloadImage(original);
    UnloadImage(canvas);
    UnloadImage(displayImg);
    UnloadImage(thumbImg);
    UnloadImageColors(pixels);
    free(r_pixels);
    free(g_pixels);
    free(b_pixels);
}