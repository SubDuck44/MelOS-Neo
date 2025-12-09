#include "main.h"

RenderTexture2D Sys_Canvas;
Texture2D Sys_Logo;
Font Sys_Font;
bool Sys_Run = true;
uint16_t Sys_WindowresX = SYS_DEF_WINDOWRESX;
uint16_t Sys_WindowresY = SYS_DEF_WINDOWRESY;
uint16_t Sys_Out = (uint16_t)-1;
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
struct Sys_Kernel {
  Vector2 origin;
  Texture2D textures[4];
} Sys_Kernel = {
  .origin = (Vector2){ SYS_DEF_WINDOWRESX / 2, SYS_DEF_WINDOWRESY / 2 }
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
  Sys_Font = LoadFontEx("res/iosevka-regular.ttf", 12, nullptr, 0);
  Sys_WindowManager.arr = malloc(sizeof(Window) * SYS_DEF_PREALLOCNUM);
  Sys_WindowManager.window_cap = SYS_DEF_PREALLOCNUM;
  Sys_WindowManager.delete_queue = malloc(sizeof(uint16_t) * SYS_DEF_PREALLOCNUM);
  Sys_WindowManager.delete_queue_cap = SYS_DEF_PREALLOCNUM;

 Sys_Kernel.textures[KER_TXT_EYE] = LoadTexture("res/kernel_eye.png");
 Sys_Kernel.textures[KER_TXT_EYE_CLSD] = LoadTexture("res/kernel_eye_closed.png");
 Sys_Kernel.textures[KER_TXT_MOUTH] = LoadTexture("res/kernel_mouth.png");
 Sys_Kernel.textures[KER_TXT_MOUTH_OPN] =  LoadTexture("res/kernel_mouth_open.png");

  sys_redrawCanvas();
}

void sys_exit(void) {
  for (uint16_t index = 0; index < Sys_WindowManager.window_num; index++) {
    Window* target = &Sys_WindowManager.arr[index];
    if (Sys_WindowManager.active_window == index) {
      UnloadRenderTexture(target->canvas);
    }
    target->kill(index, target->data);
  }
  for (uint16_t index = 0; index < sizeof(Sys_Kernel.textures) / sizeof(Texture2D); index++) {
    UnloadTexture(Sys_Kernel.textures[index]);
  }
  UnloadTexture(Sys_Logo);
  UnloadRenderTexture(Sys_Canvas);
  CloseWindow();
}

void sys_redrawCanvas(void) {
    if (Sys_WindowManager.window_num == 0) {
      BeginTextureMode(Sys_Canvas);
        ClearBackground(BLANK);
      EndTextureMode();
    } else {
      const Window* target = &Sys_WindowManager.arr[Sys_WindowManager.active_window];
      target->draw(Sys_WindowManager.active_window, target->data, target->canvas);
      BeginTextureMode(Sys_Canvas);
        ClearBackground(BLANK);
        DrawTexturePro(
        target->canvas.texture,
        (Rectangle){ 0, 0, target->res_x, (float)-target->res_y },
        (Rectangle){ 0, 0, (float)Sys_WindowresX * Sys_WindowManager.window_scale, (float)Sys_WindowresY * Sys_WindowManager.window_scale },
        (Vector2){ 0.0f, 0.0f },
        0.0f,
        WHITE
          );
      EndTextureMode();
      DrawRectangle(0, 0, Sys_WindowresX, Sys_WindowresY, RED);
    }
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

void sys_windowQueueFree(const uint16_t index) {
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

void sys_switchWindow(const uint16_t target_index) {
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
  ClearBackground(BLANK);
  if (!IsWindowMinimized()) {
    int txt_eye;
    int txt_mouth;
    switch (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      case true:
        txt_eye = KER_TXT_EYE_CLSD;
        txt_mouth = KER_TXT_MOUTH_OPN;
        break;
      case false:
        txt_eye = KER_TXT_EYE;
        txt_mouth = KER_TXT_MOUTH;
        break;
    }
    int mouse_delta_x = GetMouseX() / 64;
    int mouse_delta_y = GetMouseY() / 64;
    int offset = Sys_Kernel.textures[KER_TXT_EYE].height / 2;
    DrawTexture(
      Sys_Kernel.textures[txt_eye],
      Sys_Kernel.origin.x - 300 + mouse_delta_x - offset,
      Sys_Kernel.origin.y - 50 + mouse_delta_y - offset,
      WHITE
    );
    DrawTextureEx(
      Sys_Kernel.textures[txt_eye],
      (Vector2){
        Sys_Kernel.origin.x + 300 + mouse_delta_x + offset,
        Sys_Kernel.origin.y - 50 + mouse_delta_y + offset
      },
      180,
      1.0f,
      WHITE
    );
    DrawTexture(
      Sys_Kernel.textures[txt_mouth],
      Sys_Kernel.origin.x + mouse_delta_x - offset,
      Sys_Kernel.origin.y + 50 + mouse_delta_y - offset,
      WHITE
    );
  }
  DrawTexturePro(
    Sys_Canvas.texture,
    (Rectangle){ 0, 0, Sys_WindowresX, (float)-Sys_WindowresY },
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