#include "main.h"
#include "mods/shell.h"

#define SYS_DEF_WINDOWRESX 1280
#define SYS_DEF_WINDOWRESY 720
#define SYS_WINDOWTITLE "MelOS Neo"
#define SYS_DEF_RENDERSPEED 60
#define SYS_DEF_PREALLOCNUM 5

#define ERR_MEMALLOC "Memory allocation failed"
#define ERR_INDEXOOB "Index out of range"

RenderTexture2D Sys_Canvas;
Texture2D Sys_Logo;
bool Sys_Run = true;
uint16_t Sys_WindowresX = SYS_DEF_WINDOWRESX;
uint16_t Sys_WindowresY = SYS_DEF_WINDOWRESY;
struct Sys_WindowManager {
  Window* arr;
  uint16_t window_num;
  uint16_t window_cap;
  uint16_t active_window;
  uint16_t* delete_queue;
  uint16_t num_deletes_queued;
  uint16_t delete_queue_cap;
  float window_scale;
} Sys_WindowManager = {
  .arr = nullptr,
  .window_num = 0,
  .window_cap = 0,
  .active_window = 0,
  .delete_queue = nullptr,
  .num_deletes_queued = 0,
  .delete_queue_cap = 0,
  .window_scale = 1.0f
};

int32_t Input_Exit = KEY_ESCAPE;
int32_t Input_Alt = KEY_LEFT_ALT;


void sys_init(void) {
  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(SYS_DEF_WINDOWRESX, SYS_DEF_WINDOWRESY, SYS_WINDOWTITLE);
  SetTargetFPS(SYS_DEF_RENDERSPEED);
  ChangeDirectory("..");

  Sys_Canvas = LoadRenderTexture(SYS_DEF_WINDOWRESX, SYS_DEF_WINDOWRESY);
  Sys_Logo = LoadTexture("res/riv close.png");
  Sys_WindowManager.arr = malloc(sizeof(Window) * SYS_DEF_PREALLOCNUM);
  Sys_WindowManager.window_cap = SYS_DEF_PREALLOCNUM;
  Sys_WindowManager.delete_queue = malloc(sizeof(uint16_t) * SYS_DEF_PREALLOCNUM);
  Sys_WindowManager.delete_queue_cap = SYS_DEF_PREALLOCNUM;

  sys_redrawCanvas();
  modC_shell();
}

void sys_exit(void) {
  for (uint16_t index = 0; index < Sys_WindowManager.window_num; index++) {
    Window* target = &Sys_WindowManager.arr[index];
    if (Sys_WindowManager.active_window == index) {
      UnloadRenderTexture(target->canvas);
    }
    target->kill(index, target->data);
  }
  UnloadTexture(Sys_Logo);
  UnloadRenderTexture(Sys_Canvas);
  CloseWindow();
}

void sys_redrawCanvas(void) {
  BeginTextureMode(Sys_Canvas);
    ClearBackground(BLANK);
    if (Sys_WindowManager.window_num == 0) {
      DrawTexturePro(
        Sys_Logo,
        (Rectangle){ 0, 0, 768,  800 },
        (Rectangle){ (float)Sys_WindowresX / 2, (float)Sys_WindowresY / 2, 200, 200},
        (Vector2){ 100, 100 },
        0.0f,
        WHITE
      );
      DrawText(
        SYS_WINDOWTITLE,
        (Sys_WindowresX / 2) - (MeasureText(SYS_WINDOWTITLE, 30) / 2),
        (Sys_WindowresY / 2) + 220,
        30,
        WHITE
      );
    } else {
      const Window* target = &Sys_WindowManager.arr[Sys_WindowManager.active_window];
      target->draw(Sys_WindowManager.active_window, target->data, target->canvas);
      DrawTexturePro(
      target->canvas.texture,
      (Rectangle){ 0, 0, target->res_x, -target->res_y },
      (Rectangle){ 0, 0, Sys_WindowresX * Sys_WindowManager.window_scale, Sys_WindowresY * Sys_WindowManager.window_scale },
      (Vector2){ (float)Sys_WindowresX / 2, (float)Sys_WindowresY / 2 },
      0.0f,
      WHITE
      );
      DrawRectangle(0, 0, Sys_WindowresX, Sys_WindowresY, RED);
    }
  EndTextureMode();
}

uint16_t sys_addWindow(Window window) {
  if (Sys_WindowManager.window_num + 1 > Sys_WindowManager.window_cap) {
    Window* temp = realloc(Sys_WindowManager.arr, sizeof(Window) * (Sys_WindowManager.window_cap + SYS_DEF_PREALLOCNUM));
    if (!temp) {
      // TODO: Add own console log statement
      TraceLog(LOG_FATAL, ERR_MEMALLOC);
      Sys_Run = false;
      return -1;
    }
    Sys_WindowManager.arr = temp;
    Sys_WindowManager.window_cap += SYS_DEF_PREALLOCNUM;
  }
  Sys_WindowManager.arr[Sys_WindowManager.window_num] = window;
  Sys_WindowManager.window_num++;
  return Sys_WindowManager.window_num -1;
}

void sys_windowQueueFree(uint16_t index) {
  if (Sys_WindowManager.num_deletes_queued + 1 > Sys_WindowManager.delete_queue_cap) {
    uint16_t* temp = realloc(
      Sys_WindowManager.delete_queue, sizeof(uint16_t) * (Sys_WindowManager.delete_queue_cap + SYS_DEF_PREALLOCNUM)
    );
    if (!temp) {
      // TODO: Add own console log statement
      TraceLog(LOG_FATAL, ERR_MEMALLOC);
      Sys_Run = false;
      return;
    }
    Sys_WindowManager.delete_queue = temp;
    Sys_WindowManager.delete_queue_cap += SYS_DEF_PREALLOCNUM;
  }
  Sys_WindowManager.delete_queue[Sys_WindowManager.num_deletes_queued] = index;
  Sys_WindowManager.num_deletes_queued++;
}

void sys_switchWindow(uint16_t target_index) {
  Window* target;
  if (target_index >= Sys_WindowManager.window_num) {
    // TODO: Add own console log statment
    TraceLog(LOG_ERROR, ERR_INDEXOOB);
    return;
  } else {
    target = &Sys_WindowManager.arr[target_index];
  }
  if (Sys_WindowManager.window_num >= 1) {
    UnloadRenderTexture(Sys_WindowManager.arr[Sys_WindowManager.active_window].canvas);
  }
  target->canvas = LoadRenderTexture(target->res_x, target->res_y);
  TraceLog(LOG_INFO, "Active: %d, Target: %d", Sys_WindowManager.active_window, target_index);
  Sys_WindowManager.active_window = target_index;
}

void sys_process(void) {
  if (IsKeyPressed(Input_Exit) && IsKeyDown(Input_Alt)) Sys_Run = false;
  for (uint16_t index = 0; index < Sys_WindowManager.window_num; index++) {
    const Window* target = &Sys_WindowManager.arr[index];
    target->process(index, target->data);
  }
  if (Sys_WindowManager.num_deletes_queued <= 0) return;
  // Only queue free logic from here!
  for (uint16_t index = 0; index < Sys_WindowManager.num_deletes_queued; index++) {
    uint16_t target_index = Sys_WindowManager.delete_queue[index];
    if (target_index == Sys_WindowManager.active_window) {
      UnloadRenderTexture(Sys_WindowManager.arr[target_index].canvas);
    }
    Sys_WindowManager.arr[target_index].kill(target_index, Sys_WindowManager.arr[target_index].data);
    Sys_WindowManager.arr[target_index] = Sys_WindowManager.arr[Sys_WindowManager.window_num - 1];
    Sys_WindowManager.window_num--;
  }
  if (Sys_WindowManager.window_num < Sys_WindowManager.window_cap - SYS_DEF_PREALLOCNUM) {
    Window* temp = realloc(Sys_WindowManager.arr, sizeof(Window) * (Sys_WindowManager.window_cap - SYS_DEF_PREALLOCNUM));
    if (!temp) {
      // TODO: Add own console log statement
      TraceLog(LOG_FATAL, ERR_MEMALLOC);
      Sys_Run = false;
      return;
    }
    Sys_WindowManager.arr = temp;
    Sys_WindowManager.window_cap -= SYS_DEF_PREALLOCNUM;
  }
  if (Sys_WindowManager.num_deletes_queued <= Sys_WindowManager.delete_queue_cap - SYS_DEF_PREALLOCNUM) {
    uint16_t* temp = realloc(
      Sys_WindowManager.delete_queue, sizeof(uint16_t) * (Sys_WindowManager.delete_queue_cap - SYS_DEF_PREALLOCNUM)
    );
    if (!temp) {
      // TODO: Add own console log statment
      TraceLog(LOG_FATAL, ERR_MEMALLOC);
      Sys_Run = false;
      return;
    }
    Sys_WindowManager.delete_queue = temp;
    Sys_WindowManager.delete_queue_cap -= SYS_DEF_PREALLOCNUM;
  }
  Sys_WindowManager.num_deletes_queued = 0;
}

void sys_draw(void) {
  BeginDrawing();
  DrawTexturePro(
    Sys_Canvas.texture,
    (Rectangle){ 0, 0, Sys_WindowresX, -Sys_WindowresY },
    (Rectangle){ 0, 0, Sys_WindowresX, Sys_WindowresY },
    (Vector2){ 0, 0 },
    0.0f,
    WHITE
  );
  EndDrawing();
}

int32_t main(void) {
  sys_init();
  while (Sys_Run) {
    sys_process();
    sys_draw();
  }
  sys_exit();
  return 0;
}