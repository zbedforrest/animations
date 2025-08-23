#ifndef GLOBALS_H
#define GLOBALS_H

#include "raylib.h"

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
    Image recreationImage;
    int z_offset;
    int stripe_height;
    double leftKeyDownTime;
    double rightKeyDownTime;

    // Animation State
    bool animating;
    float t;
    float dt;
    Rectangle animateButton;

} AppState;

#endif // GLOBALS_H
