#include "main.h"
#include "mods/shell.h"
#include <raylib.h>

/* SYSTEM */
bool sys_run_flag = true;
bool sys_redraw_flag = true;
RenderTexture2D sys_canvas;
uint16_t sys_windowres_x = 1920;
uint16_t sys_windowres_y = 1000;
Font sys_font;

/* KEYMAP */
int32_t key_exit = KEY_ESCAPE;
int32_t key_alt = KEY_LEFT_ALT;

struct Shell Sys_Shell;
struct ProcessQueue Sys_ProcQueue;

void Sys_Abort(const char* error) {
  TraceLog(LOG_FATAL, error);
  Sys_QueueFlush(false);
  sys_run_flag = false;
}

void Sys_Invoke(Method op, void* args) {
  if (Sys_ProcQueue.len + 1 > Sys_ProcQueue.cap) {
    ProcessInstruction* temp = realloc(
      Sys_ProcQueue.q,
      sizeof(ProcessInstruction) * (Sys_ProcQueue.len + DEF_PREALLOCSIZE)
    );
    if (!temp) {
      Sys_Abort(ERR_MEMALLOC);
    }
    Sys_ProcQueue.q = temp;
    Sys_ProcQueue.cap += DEF_PREALLOCSIZE;
    Sys_ProcQueue.cache_timeout = DEF_CACHETIMEOUT;
  }
  Sys_ProcQueue.q[Sys_ProcQueue.len] = (ProcessInstruction){
    op, args
  };
  Sys_ProcQueue.len++;
}

void Sys_QueueFlush(bool clear_cache) {
  Sys_ProcQueue.len = 0;
  if (clear_cache) {
    ProcessInstruction* temp = realloc(
      Sys_ProcQueue.q,
      sizeof(ProcessInstruction) * (DEF_PREALLOCSIZE)
    );
    if (!temp) {
      Sys_Abort(ERR_MEMALLOC);
    }
    Sys_ProcQueue.q = temp;
    Sys_ProcQueue.cap -= DEF_PREALLOCSIZE;
  }
}

void Sys_QueueExecute(void) {
  for (uint16_t i = 0; i < Sys_ProcQueue.len; i++) {
    Sys_ProcQueue.q[i].op(Sys_ProcQueue.q[i].args);
  }
  if (Sys_ProcQueue.len <= Sys_ProcQueue.cap - DEF_PREALLOCSIZE) {
    Sys_ProcQueue.cache_timeout--;
    if (Sys_ProcQueue.cache_timeout <= 0 && Sys_ProcQueue.cap > DEF_PREALLOCSIZE) {
      ProcessInstruction* temp = realloc(
        Sys_ProcQueue.q,
        sizeof(ProcessInstruction) * (Sys_ProcQueue.cap - DEF_PREALLOCSIZE)
      );
      if (!temp) {
        Sys_Abort(ERR_MEMALLOC);
      }
      Sys_ProcQueue.q = temp;
      Sys_ProcQueue.cap -= DEF_PREALLOCSIZE;
    }
  } else {
    Sys_ProcQueue.cache_timeout = DEF_CACHETIMEOUT;
  }
  Sys_ProcQueue.len = 0;
}

void Sys_RedrawCanvas(void) {
  BeginTextureMode(sys_canvas);
    ClearBackground(DEF_BACKDROPCOLOR);
    Shell_Draw(&Sys_Shell);
  EndTextureMode();
  sys_redraw_flag = false;
}

void Sys_Process(void) {
  if (IsKeyPressed(key_exit) && IsKeyDown(key_alt)) sys_run_flag = false;
  Sys_QueueExecute();
  if (GetKeyPressed()) Shell_OnInput(&Sys_Shell);
}

void Sys_Draw(void) {
  if (sys_redraw_flag) Sys_RedrawCanvas();
  BeginDrawing();
    DrawTexturePro(
      sys_canvas.texture,
      (Rectangle){ 0, 0, sys_windowres_x, (float)-sys_windowres_y },
      (Rectangle){ 0, 0, sys_windowres_x, sys_windowres_y },
      (Vector2){ 0, 0 },
      0.0f,
      WHITE
    );
  EndDrawing();
}

void Sys_Init(void) {
  InitWindow(sys_windowres_x, sys_windowres_y, SYS_TITLE);
  SetTargetFPS(DEF_RENDERSPEED);
  SetWindowPosition(0, 0);

  sys_canvas = LoadRenderTexture(sys_windowres_x, sys_windowres_y);

  sys_font = LoadFont("res/iosevka-regular.ttf");

  Sys_Shell = Shell_Construct();
  Sys_ProcQueue = (struct ProcessQueue){
    .cache_timeout = 0,
    .cap = DEF_PREALLOCSIZE,
    .len = 0,
    .q = malloc(sizeof(ProcessInstruction) * DEF_PREALLOCSIZE)
  };
  Shell_Print(&Sys_Shell, "Hello World!", sizeof("Hello World!"), true);
}

void Sys_Exit(void) {
  UnloadRenderTexture(sys_canvas);
  UnloadFont(sys_font);
  CloseWindow();
}

int main(void) {
  Sys_Init();
  while (sys_run_flag) {
    Sys_Process();
    Sys_Draw();
  }
  Sys_Exit();
  return 0;
}