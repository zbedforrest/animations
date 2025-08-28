#include "view_recreate.h"
#include "raylib.h"
#include <stdio.h> // For TextFormat
#include <stdlib.h> // For malloc/free
#include <string.h> // For string functions

//----------------------------------------------------------------------------------
// Module Local Functions Declaration
//----------------------------------------------------------------------------------
static void RegenerateRecreationImage(AppState *state);
static void StartVideoRecording(AppState *state);
static void StopVideoRecording(AppState *state);
static void CaptureFrame(AppState *state);
static void ExportFramesToVideo(AppState *state);

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
void UpdateRecreateView(AppState *state)
{
    bool z_changed = false;

    // Return to analyzer
    if (IsKeyPressed(KEY_A)) {
        state->currentView = VIEW_ANALYZER;
        state->animating = false; // Stop animation when leaving view
        SetWindowSize(1400, 800);
    }

    // Toggle animation
    if (IsKeyPressed(KEY_SPACE)) {
        state->animating = !state->animating;
    }

    // Video recording controls
    if (IsKeyPressed(KEY_R)) {
        if (!state->recording) {
            StartVideoRecording(state);
        } else {
            StopVideoRecording(state);
        }
    }

    // Update t if animating
    if (state->animating) {
        state->t += state->dt;
        RegenerateRecreationImage(state); // Regenerate on each frame for animation
        UpdateTexture(state->recreationTexture, state->recreationImage.data);
    }

    // Handle LEFT key
    if (IsKeyPressed(KEY_LEFT)) {
        state->z_offset--;
        z_changed = true;
        state->leftKeyDownTime = GetTime();
    }
    if (IsKeyDown(KEY_LEFT) && state->leftKeyDownTime > 0.0) {
        if ((GetTime() - state->leftKeyDownTime) > 0.5) {
            state->z_offset -= (int)(100.0f * GetFrameTime());
            z_changed = true;
        }
    }
    if (IsKeyReleased(KEY_LEFT)) {
        state->leftKeyDownTime = 0.0;
    }

    // Handle RIGHT key
    if (IsKeyPressed(KEY_RIGHT)) {
        state->z_offset++;
        z_changed = true;
        state->rightKeyDownTime = GetTime();
    }
    if (IsKeyDown(KEY_RIGHT) && state->rightKeyDownTime > 0.0) {
        if ((GetTime() - state->rightKeyDownTime) > 0.5) {
            state->z_offset += (int)(100.0f * GetFrameTime());
            z_changed = true;
        }
    }
    if (IsKeyReleased(KEY_RIGHT)) {
        state->rightKeyDownTime = 0.0;
    }

    // If Z changed, regenerate the image data and update the GPU texture
    if (z_changed && !state->animating) { // Avoid double-update when animating
        RegenerateRecreationImage(state);
        UpdateTexture(state->recreationTexture, state->recreationImage.data);
    }
}

void DrawRecreateView(AppState *state)
{
    BeginDrawing();
        ClearBackground(BLACK);
        if (state->recreationTexture.id > 0) {
            DrawTexturePro(state->recreationTexture, (Rectangle){ 0, 0, (float)state->recreationTexture.width, (float)state->recreationTexture.height },
                           (Rectangle){ 0, 0, 1000, 800 }, (Vector2){ 0, 0 }, 0.0f, WHITE);
        }
        
    EndDrawing();
    
    // Capture frame if recording
    if (state->recording && state->animating) {
        CaptureFrame(state);
    }
}

//----------------------------------------------------------------------------------
// Module Local Functions Definition
//----------------------------------------------------------------------------------
static unsigned char get_animated_r(AppState *state, int x, bool reversed) {
    int width = state->recreationImage.width;
    int sourceX_r;
    if (reversed) {
        sourceX_r = ((x + state->z_offset - (int)state->t) % width + width) % width;
    } else {
        sourceX_r = ((x + state->z_offset + (int)state->t) % width + width) % width;
    }
    return state->keyframe_pixels[sourceX_r].r;
}

static unsigned char get_animated_g(AppState *state, int x, bool reversed) {
    int width = state->recreationImage.width;
    int sourceX_g;
    if (reversed) {
        sourceX_g = ((x + state->z_offset - (int)state->t) % width + width) % width;
    } else {
        sourceX_g = ((x + state->z_offset + (int)state->t) % width + width) % width;
    }
    return state->keyframe_pixels[sourceX_g].g;
}

static unsigned char get_animated_b(AppState *state, int x, bool reversed) {
    int width = state->recreationImage.width;
    int sourceX_b;
    if (reversed) {
        sourceX_b = ((x + state->z_offset - (int)state->t) % width + width) % width;
    } else {
        sourceX_b = ((x + state->z_offset + (int)state->t) % width + width) % width;
    }
    return state->keyframe_pixels[sourceX_b].b;
}

static void RegenerateRecreationImage(AppState *state)
{
    Color *newPixels = (Color *)state->recreationImage.data;
    int width = state->recreationImage.width;
    int height = state->recreationImage.height;

    for (int y = 0; y < height; y++) {
        bool reversed = ((y / state->stripe_height) % 2 != 0);
        for (int x = 0; x < width; x++) {
            Color *pixel = &newPixels[y * width + x];
            if (state->animating) {
                pixel->r = get_animated_r(state, x, reversed);
                pixel->g = get_animated_g(state, x, reversed);
                pixel->b = get_animated_b(state, x, reversed);
                pixel->a = 255;
            } else {
                int sourceX;
                if (reversed) {
                    sourceX = ( ( (width - 1 - x) + state->z_offset) % width + width ) % width;
                } else {
                    sourceX = x;
                }
                *pixel = state->keyframe_pixels[sourceX];
            }
        }
    }
}

//----------------------------------------------------------------------------------
// Video Recording Functions
//----------------------------------------------------------------------------------
static void StartVideoRecording(AppState *state)
{
    if (state->recording) return;
    
    printf("Starting video recording...\n");
    state->recording = true;
    state->frame_count = 0;
    
    // Allocate frame buffer
    state->frame_buffer = (Image *)malloc(state->max_frames * sizeof(Image));
    if (!state->frame_buffer) {
        printf("Failed to allocate frame buffer!\n");
        state->recording = false;
        return;
    }
    
    // Start animation if not already running
    if (!state->animating) {
        state->animating = true;
        state->t = 0.0f; // Reset animation time
    }
    
    printf("Video recording started. Press R again to stop.\n");
}

static void StopVideoRecording(AppState *state)
{
    if (!state->recording) return;
    
    printf("Stopping video recording...\n");
    state->recording = false;
    
    if (state->frame_count > 0) {
        printf("Captured %d frames. Exporting video...\n", state->frame_count);
        ExportFramesToVideo(state);
    }
    
    // Cleanup frame buffer
    if (state->frame_buffer) {
        for (int i = 0; i < state->frame_count; i++) {
            UnloadImage(state->frame_buffer[i]);
        }
        free(state->frame_buffer);
        state->frame_buffer = NULL;
    }
    
    state->frame_count = 0;
    printf("Video recording stopped.\n");
}

static void CaptureFrame(AppState *state)
{
    if (!state->recording || !state->frame_buffer) return;
    if (state->frame_count >= state->max_frames) {
        printf("Maximum frames reached. Stopping recording.\n");
        StopVideoRecording(state);
        return;
    }
    
    // Capture the current screen
    state->frame_buffer[state->frame_count] = LoadImageFromScreen();
    state->frame_count++;
}

static void ExportFramesToVideo(AppState *state)
{
    if (!state->frame_buffer || state->frame_count == 0) return;
    
    // Create frames directory
    char command[512];
    snprintf(command, sizeof(command), "mkdir -p %s", state->recording_dir);
    system(command);
    
    // Export frames as PNG files
    printf("Exporting %d frames to %s/\n", state->frame_count, state->recording_dir);
    for (int i = 0; i < state->frame_count; i++) {
        char filename[512];
        snprintf(filename, sizeof(filename), "%s/frame_%06d.png", state->recording_dir, i);
        ExportImage(state->frame_buffer[i], filename);
        
        if (i % 30 == 0) { // Progress indicator every 30 frames
            printf("Exported frame %d/%d\n", i + 1, state->frame_count);
        }
    }
    
    // Generate ffmpeg command
    char ffmpeg_cmd[1024];
    snprintf(ffmpeg_cmd, sizeof(ffmpeg_cmd), 
        "ffmpeg -y -framerate 30 -i %s/frame_%%06d.png -c:v libx264 -pix_fmt yuv420p -crf 18 animation_output.mp4",
        state->recording_dir);
    
    printf("\nFrames exported successfully!\n");
    printf("To create the video, run this command:\n");
    printf("%s\n", ffmpeg_cmd);
    printf("\nOr running it automatically...\n");
    
    // Try to run ffmpeg automatically
    int result = system(ffmpeg_cmd);
    if (result == 0) {
        printf("Video created successfully: animation_output.mp4\n");
        
        // Cleanup frames directory
        printf("Cleaning up temporary frames...\n");
        char cleanup_cmd[512];
        #ifdef _WIN32
        snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rmdir /s /q %s", state->recording_dir);
        #else
        snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s", state->recording_dir);
        #endif
        system(cleanup_cmd);
    } else {
        printf("ffmpeg command failed. You can manually run the command above.\n");
        printf("Make sure ffmpeg is installed and in your PATH.\n");
    }
}