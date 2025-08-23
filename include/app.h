#ifndef APP_H
#define APP_H

#include "globals.h"

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
void InitApp(AppState *state, const char *filename);
void RunApp(AppState *state);
void CleanupApp(AppState *state);

#endif // APP_H
