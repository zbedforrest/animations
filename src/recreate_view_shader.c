#include "recreate_view_shader.h"
#include "globals.h"
#include "raylib.h"
#include <stdlib.h>


static int frameCount = 0;


RecreateShaderView *RecreateShaderView_Init(AppState *state) {
  RecreateShaderView *view = malloc(sizeof(RecreateShaderView));
  if (view == NULL) {
    return NULL;
  }

  view->texture = LoadTexture("assets/TARGET5.png");   // Load model texture (diffuse map)
  

  view->shader = LoadShader(0, "src/recreate_view_shader.fs");
  view->swirlCenterLoc = GetShaderLocation(view->shader, "center");
  view->swirlCenter[0] = (float)state->finalWidth/2;
  view->swirlCenter[1] = (float)state->finalHeight/2;
  SetShaderValue(view->shader, view->swirlCenterLoc, view->swirlCenter, SHADER_UNIFORM_VEC2);
  // Create a RenderTexture2D to be used for render to texture
  view->target = LoadRenderTexture(view->texture.width, view->texture.height);

  // Get shader uniform locations
  view->timeLoc = GetShaderLocation(view->shader, "u_time");
  view->resolutionLoc = GetShaderLocation(view->shader, "u_resolution");

  // Set the resolution uniform
  float resolution[2] = {(float)GetScreenWidth(), (float)GetScreenHeight()};
  SetShaderValue(view->shader, view->resolutionLoc, resolution,
                 SHADER_UNIFORM_VEC2);

  return view;
}

void RecreateShaderView_Update(RecreateShaderView *view, AppState *state) {
  // Handle view switching
  if (IsKeyPressed(KEY_RIGHT)) {
    state->currentView = VIEW_ANALYZER;
  }
  if (IsKeyPressed(KEY_LEFT)) {
    state->currentView = VIEW_RECREATE;
  }

  // Update the time uniform for the animation
  float time = (float)GetTime();
  SetShaderValue(view->shader, view->timeLoc, &time, SHADER_UNIFORM_FLOAT);
}

void RecreateShaderView_Draw(RecreateShaderView *view, AppState *state) {
  (void)state; // Unused

  Vector2 mousePosition = GetMousePosition();
  view->swirlCenter[0] = mousePosition.x;
  view->swirlCenter[1] = GetScreenHeight() - mousePosition.y;
  SetShaderValue(view->shader, view->swirlCenterLoc, view->swirlCenter, SHADER_UNIFORM_VEC2);


  BeginTextureMode(view->target);       // Enable drawing to texture
    ClearBackground(RAYWHITE);  // Clear texture background
    DrawTexture(view->texture, 0, 0, WHITE);
    DrawText("TEXT DRAWN IN RENDER TEXTURE", 200, 10, 30, RED);
  EndTextureMode();

  BeginDrawing();
  ClearBackground(BLACK);

  // // Use the shader to draw on the whole screen
  BeginShaderMode(view->shader);
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), WHITE);
    DrawTexture(view->target.texture,  0,0 , WHITE);
  EndShaderMode();

  // DrawFPS(10, 10);
  EndDrawing();

  // After the first frame is drawn, capture screenshot and exit
  frameCount++;
  if (IsKeyPressed(KEY_P)) { // Wait for at least 2 frames to ensure everything is rendered
    TakeScreenshot("output_screenshot.png");
    exit(0); // Close the program
  }
}

void RecreateShaderView_Exit(RecreateShaderView *view) {
  UnloadShader(view->shader);
  free(view);
}