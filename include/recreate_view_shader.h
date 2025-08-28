#ifndef RECREATE_VIEW_SHADER_H
#define RECREATE_VIEW_SHADER_H

#include "globals.h"
#include "raylib.h"

typedef struct RecreateShaderView {
  Shader shader;
  int timeLoc;
  int resolutionLoc;
  RenderTexture2D target;
  Texture2D texture;
  int swirlCenterLoc;
  float swirlCenter[2];
} RecreateShaderView;

RecreateShaderView *RecreateShaderView_Init(AppState *state);
void RecreateShaderView_Update(RecreateShaderView *view, AppState *state);
void RecreateShaderView_Draw(RecreateShaderView *view, AppState *state);
void RecreateShaderView_Exit(RecreateShaderView *view);

#endif // RECREATE_VIEW_SHADER_H
