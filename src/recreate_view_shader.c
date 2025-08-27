#include "recreate_view_shader.h"
#include "globals.h"
#include "raylib.h"
#include <stdlib.h>

RecreateShaderView *RecreateShaderView_Init(void) {
  RecreateShaderView *view = malloc(sizeof(RecreateShaderView));
  if (view == NULL) {
    return NULL;
  }

  // Load the glow ring shader
  view->shader = LoadShader(0, "src/glow_ring.fs");

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

  BeginDrawing();
  ClearBackground(BLACK);

  // Use the shader to draw on the whole screen
  BeginShaderMode(view->shader);
  DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), WHITE);
  EndShaderMode();

  DrawFPS(10, 10);
  EndDrawing();
}

void RecreateShaderView_Exit(RecreateShaderView *view) {
  UnloadShader(view->shader);
  free(view);
}