#include "main.h"
#include "mods/shell.h"

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

void Sys_RedrawCanvas(void) {
  BeginTextureMode(sys_canvas);
    ClearBackground(BLANK);
    Shell_Draw(&Sys_Shell);
  EndTextureMode();
  sys_redraw_flag = false;
}

void Sys_Process(void) {
  if (IsKeyPressed(key_exit) && IsKeyDown(key_alt)) sys_run_flag = false;
  if (IsKeyPressed(KEY_SPACE)) Shell_Print(&Sys_Shell, "B00B5", 5);
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
  Shell_Print(&Sys_Shell, "Hello World!", sizeof("Hello World!"));
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