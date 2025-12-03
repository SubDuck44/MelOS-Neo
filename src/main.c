#include "raylib.h"
#include <stdint.h>

#include <stdlib.h>
#include <string.h>

// Defines
#define SYS_DEF_WND_RES_X 1280
#define SYS_DEF_WND_RES_Y 720
#define SYS_WND_TITLE "MelOS Neo"
#define SYS_DEF_RENDER_SPEED 60
#define SYS_PREALLOC_NUM 5

// Enums
enum ExitCodes {
  ERR = -1,
  OK = 0
};
enum ErrorCodes {
  ERR_MEMALLOC,
  ERR_INDEX_OOB,
  ERR_INVALIDSIZE
};

// Types
typedef struct {
  int32_t X;
  int32_t Y;
} Vector2i;
typedef struct {
  void* Arr;
  uint16_t Len;
  uint16_t Cap;
  size_t BlockSize;
} Sys_Queue;
typedef void(*Method)(Sys_Queue* Owner, uint16_t Index, void* Data);
typedef struct {
  Method Update;
  Method Draw;
  void* Data;
} Sys_Window;

// Global system variables
bool Sys_Run = true;
RenderTexture2D Sys_Canvas = { 0 };
Vector2i Sys_WindowRes = { SYS_DEF_WND_RES_X, SYS_DEF_WND_RES_Y };
Sys_Queue Sys_Windows;
uint16_t Sys_ActiveWindow = 0;

// Global keymap
int32_t Key_Exit = KEY_ESCAPE;
int32_t Key_Alt = KEY_LEFT_ALT;

// Sys Functions
void sys_redrawcanvas(void);

void sys_init(void) {
  // Window init via RLGL
  SetConfigFlags(FLAG_MSAA_4X_HINT); // Enable Anti-Aliasing
  InitWindow(SYS_DEF_WND_RES_X, SYS_DEF_WND_RES_Y, SYS_WND_TITLE);
  SetTargetFPS(SYS_DEF_RENDER_SPEED);
  // Sys globals init
  Sys_Run = true;
  Sys_Canvas = LoadRenderTexture(SYS_DEF_WND_RES_X, SYS_DEF_WND_RES_Y);
  Sys_Windows = (Sys_Queue){
    malloc(sizeof(Sys_Window) * SYS_PREALLOC_NUM),
    0,
    5,
    sizeof(Sys_Window)
  };
  // Run external init calls
  sys_redrawcanvas();
}

void sys_exit(void) {
  CloseWindow();
}

void sys_redrawcanvas(void) {
  Sys_Window* target = { nullptr };
  BeginTextureMode(Sys_Canvas);
    ClearBackground(BLANK);
    // Update windows
    for (uint16_t i = 0; i < Sys_Windows.Len; i++) {
      target = Sys_Windows.Arr + (Sys_ActiveWindow * Sys_Windows.BlockSize);
      target->Update(&Sys_Windows, i, target->Data);
    }
    // Draw active window
    if (Sys_ActiveWindow != (uint16_t)-1) {
      target = Sys_Windows.Arr + (Sys_ActiveWindow * Sys_Windows.BlockSize);
      target->Draw(&Sys_Windows, Sys_ActiveWindow, target->Data);
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

void sys_queue_append(Sys_Queue* Target, void* Data, size_t Size) {
  if (Target->BlockSize != Size) {
    TraceLog(LOG_ERROR, "%d", ERR_INVALIDSIZE);
    return;
  }
  if (Target->Len + 1 > Target->Cap) {
    void* temp = realloc(Target->Arr, (Target->Cap + SYS_PREALLOC_NUM) * Size);
    if (!temp) {
      TraceLog(LOG_FATAL, "%d", ERR_MEMALLOC);
      return;
    }
    Target->Arr = temp;
    Target->Cap += SYS_PREALLOC_NUM;
  }
  memcpy(Target->Arr + (Target->Len + Target->BlockSize), Data, Size);
  Target->Len++;
}

void sys_queue_pop(Sys_Queue* Target, uint16_t Index, size_t Size) {
  if (Target->BlockSize != Size) {
    TraceLog(LOG_ERROR, "%d", ERR_INVALIDSIZE);
    return;
  }
  if (Index != Target->Len - 1) {
    memcpy(Target->Arr + (Index * Target->BlockSize), Target->Arr + (Target->Len - 1 * (Target->BlockSize)), Size);
  }
  Target->Len--;
  if (Target->Len < Target->Cap - SYS_PREALLOC_NUM && Target->Cap > SYS_PREALLOC_NUM) {
    void* temp = realloc(Target->Arr, (Target->Cap - SYS_PREALLOC_NUM) * Target->BlockSize);
    if (!temp) {
      TraceLog(LOG_FATAL, "%d", ERR_MEMALLOC);
      return;
    }
    Target->Arr = temp;
    Target->Cap -= SYS_PREALLOC_NUM;
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