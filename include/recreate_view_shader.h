#ifndef RECREATE_VIEW_SHADER_H
#define RECREATE_VIEW_SHADER_H

#include "globals.h"
#include "raylib.h"

typedef struct RecreateShaderView {
  Shader shader;
  int timeLoc;
  int resolutionLoc;
} RecreateShaderView;

RecreateShaderView *RecreateShaderView_Init(void);
void RecreateShaderView_Update(RecreateShaderView *view, AppState *state);
void RecreateShaderView_Draw(RecreateShaderView *view, AppState *state);
void RecreateShaderView_Exit(RecreateShaderView *view);

#endif // RECREATE_VIEW_SHADER_H
