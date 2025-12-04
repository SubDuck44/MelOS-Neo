#include "main.h"

// Defines
#define SYS_DEF_WND_RES_X 1280
#define SYS_DEF_WND_RES_Y 720
#define SYS_WND_TITLE "MelOS Neo"
#define SYS_DEF_RENDER_SPEED 60
#define SYS_PREALLOC_NUM 5

// Global system variables
bool Sys_Run = true;
RenderTexture2D Sys_Canvas = { 0 };
Vector2i Sys_WindowRes = { SYS_DEF_WND_RES_X, SYS_DEF_WND_RES_Y };
Sys_WindowQueue Sys_Windows;
uint16_t Sys_ActiveWindow = 0;
Font Sys_Font = {0};

// Global keymap
int32_t Key_Exit = KEY_ESCAPE;
int32_t Key_Alt = KEY_LEFT_ALT;

// Sys Functions
void sys_init(void) {
  // Window init via RLGL
  SetConfigFlags(FLAG_MSAA_4X_HINT); // Enable Anti-Aliasing
  InitWindow(SYS_DEF_WND_RES_X, SYS_DEF_WND_RES_Y, SYS_WND_TITLE);
  SetTargetFPS(SYS_DEF_RENDER_SPEED);
  // Sys globals init
  Sys_Run = true;
  Sys_Canvas = LoadRenderTexture(SYS_DEF_WND_RES_X, SYS_DEF_WND_RES_Y);
  Sys_Windows = (Sys_WindowQueue){
    malloc(sizeof(Sys_Window) * SYS_PREALLOC_NUM),
    0,
    5
  };
  Sys_ActiveWindow = (uint16_t)-1;
  Sys_Font = LoadFont("res/iosevka-regular.ttf");
  // Run external init calls
  sys_redrawcanvas();
}

void sys_exit(void) {
  CloseWindow();
}

void sys_switchwindow(uint16_t Target) {
  if (Target < 0 || Target >= Sys_Windows.Len) {
    TraceLog(LOG_ERROR, "%d", ERR_INDEX_OOB);
    return;
  }
  if (Sys_ActiveWindow != (uint16_t)-1) {
    UnloadRenderTexture(Sys_Windows.Arr[Sys_ActiveWindow].Canvas);
  }
  Sys_ActiveWindow = Target;
  Sys_Windows.Arr[Sys_ActiveWindow].Canvas = LoadRenderTexture((int32_t)Sys_WindowRes.X, (int32_t)Sys_WindowRes.Y);
}

void sys_redrawcanvas(void) {
  BeginTextureMode(Sys_Canvas);
    ClearBackground(BLANK);
    // Update windows
    for (uint16_t i = 0; i < Sys_Windows.Len; i++) {
      Sys_Windows.Arr[i].Update(i, Sys_Windows.Arr[i].Data);
    }
    // Draw active window
    if (Sys_ActiveWindow != (uint16_t)-1) {
      Sys_Window* target = &Sys_Windows.Arr[Sys_ActiveWindow];
      target->Draw(target->Canvas, Sys_ActiveWindow, target->Data);
      DrawTexturePro(
        target->Canvas.texture,
        (Rectangle){0, 0, (float)target->Canvas.texture.width, (float)target->Canvas.texture.height},
        (Rectangle){0, 0, (float)Sys_WindowRes.X, (float)Sys_WindowRes.Y},
        (Vector2){0, 0},
        0.0f,
        WHITE
      );
    }
  EndTextureMode();
}

void sys_onframe(void) {
  // Global interrupt checks
  if (IsKeyPressed(Key_Exit) && IsKeyDown(Key_Alt)) Sys_Run = false;
  // Draw Main Canvas
  BeginDrawing();
    DrawTexturePro(
      Sys_Canvas.texture,
      (Rectangle){ 0, 0, (float)Sys_WindowRes.X, (float)-Sys_WindowRes.Y },
      (Rectangle){ 0, 0, (float)Sys_WindowRes.X, (float)Sys_WindowRes.Y },
      (Vector2){ 0.0f, 0.0f },
      0.0f,
      WHITE
    );
  EndDrawing();
}

void sys_window_append(Sys_Window Target) {
  if (Sys_Windows.Len + 1 > Sys_Windows.Cap) {
    Sys_Window* temp = realloc(Sys_Windows.Arr, sizeof(Sys_Window * (Sys_Windows.Cap + SYS_PREALLOC_NUM)));
    if (!temp) {
      TraceLog(LOG_FATAL, "%d", ERR_MEMALLOC);
      return;
    }
    Sys_Windows.Arr = temp;
    Sys_Windows.Cap += SYS_PREALLOC_NUM;
  }
  Sys_Windows.Arr[Sys_Windows.Len] = Target;
  Sys_Windows.Len++;
}

void sys_window_pop(uint16_t Index) {
  if (Index < 0 || Index >= Sys_Windows.Len) {
    TraceLog(LOG_ERROR, "%d", ERR_INDEX_OOB);
    return;
  }
  if (Index != Sys_Windows.Len - 1) {
    Sys_Windows.Arr[Index] = Sys_Windows.Arr[Sys_Windows.Len - 1];
  }
  Sys_Windows.Len--;
  if (Sys_Windows.Len <= Sys_Windows.Cap - SYS_PREALLOC_NUM) {
    Sys_Window* temp = realloc(Sys_Windows.Arr, sizeof(Sys_Window) * (Sys_Windows.Cap - SYS_PREALLOC_NUM));
    if (!temp) {
      TraceLog(LOG_FATAL, "%d", ERR_MEMALLOC);
      return;
    }
    Sys_Windows.Arr = temp;
  }
}

// ENTRY POINT
int32_t main(void) {
  sys_init();
  while (Sys_Run) {
    sys_onframe();
  }
  sys_exit();
  return OK;
}