#include "main.h"
#include "mods/shell.h"
#include <raylib.h>

/* SYSTEM */
bool sys_run_flag = true;
RenderTexture2D sys_canvas;
uint16_t sys_windowres_x = 1920;
uint16_t sys_windowres_y = 1000;
Font sys_font;

/* KEYMAP */
int32_t key_exit = KEY_ESCAPE;
int32_t key_alt = KEY_LEFT_ALT;

struct Shell Sys_Shell;
struct Runtime Sys_Runtime;
struct WindowManager Sys_WindowManager;

uint16_t WM_CreateWindow(void) {
  Sys_WindowManager.next_id++;
  return Sys_WindowManager.next_id - 1;
}

void WM_Draw(Method method, void* args, uint16_t index) {
  if (Sys_WindowManager.active_window == index) {
    BeginTextureMode(sys_canvas);
      method(args);
    EndTextureMode();
  }
}

void Sys_Abort(void) {
  Sys_QueueFlush(false);
  sys_run_flag = false;
}

void Sys_QueueFlush(const bool clear_cache) {
  if (clear_cache) {
    uint8_t* temp = realloc(Sys_Runtime.q, sizeof(uint8_t) * DEF_PREALLOCSIZE);
    Sys_Runtime.cap = DEF_PREALLOCSIZE;
    Sys_Runtime.q = temp;
  }
  Sys_Runtime.len = 0;
}

void Sys_Invoke(const uint8_t opcode, void* args) {
  uint16_t args_size;
  switch (opcode) {
    case OP_PROCESSCALL:
      args_size = sizeof(struct Op_ProcessCall);
      break;
    case OP_DRAWCALL:
      args_size = sizeof(struct Op_DrawCall);
      break;
    #ifndef NDEBUG
    default:
      TraceLog(LOG_ERROR, ERR_INVALIDOPCODE, opcode);
      return;
    #endif
  }
  if (Sys_Runtime.len + 1 + args_size > Sys_Runtime.cap) {
    uint8_t* temp = realloc(Sys_Runtime.q, sizeof(uint8_t) * (Sys_Runtime.cap + DEF_PREALLOCSIZE));
    if (!temp) {
      TraceLog(LOG_FATAL, ERR_MEMALLOC);
      Sys_Abort();
      return;
    }
    Sys_Runtime.q = temp;
    Sys_Runtime.cap += DEF_PREALLOCSIZE;
  }
  Sys_Runtime.q[Sys_Runtime.len] = opcode;
  memcpy(Sys_Runtime.q + (Sys_Runtime.len + 1), args, args_size);
  Sys_Runtime.len += 1 + args_size;
}

void Sys_ExecuteQueue(void) {
  bool redrawn = false;
  for (uint16_t index = 0; index < Sys_Runtime.len;) {
    switch (Sys_Runtime.q[index]) {
      case OP_PROCESSCALL:
        {
          struct Op_ProcessCall* data = (void*)&Sys_Runtime.q[index + 1];
          data->method(data->args);
          index += sizeof(struct Op_ProcessCall) + 1;
        }
        break;
      case OP_DRAWCALL:
        {
          struct Op_DrawCall* data = (void*)&Sys_Runtime.q[index + 1];
          if (!redrawn) {
            BeginTextureMode(sys_canvas);
              ClearBackground(DEF_BACKDROPCOLOR);
            EndTextureMode();
            redrawn = true;
          }
          WM_Draw(data->method, data->args, data->index);
          index += sizeof(struct Op_DrawCall) + 1;
        }
        break;
      #ifndef NDEBUG
      default:
        TraceLog(LOG_ERROR, ERR_INVALIDOPCODE, Sys_Runtime.q[index]);
        return;
      #endif
    }
  }
  Sys_Runtime.len = 0;
}

void Sys_Process(void) {
  if (IsKeyPressed(key_exit) && IsKeyDown(key_alt)) sys_run_flag = false;
  Sys_ExecuteQueue();
  if (GetKeyPressed()) Shell_OnInput(&Sys_Shell);
}

void Sys_Draw(void) {
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
  Sys_Runtime = (struct Runtime) {
    .q = calloc(DEF_PREALLOCSIZE, sizeof(char)),
    .cap = DEF_RENDERSPEED,
    .len = 0,
    .cache_timeout = 0
  };
  Sys_WindowManager = (struct WindowManager){
    .active_window = 0,
    .next_id = 0
  };
  Sys_Shell = Shell_Construct();

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