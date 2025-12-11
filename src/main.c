#include "main.h"

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

/* SHELL */
char* shell_buffer;
char* shell_lastmessage = (char*)(&(shell_buffer) + 1);
uint8_t shell_fontsize = 20;
float shell_spacing = 3;

void Shell_Print(const char* message, size_tits len) {
  shell_lastmessage = mstr_lencpy(message, shell_lastmessage + 2, len);
  sys_redraw_flag = true;
  // No way this works.
  // Update: IT FUCKING WORKS!!!
}

void Shell_Draw(void) {
  uint16_t draw_y = sys_windowres_y - shell_fontsize;
  uint16_t draw_x = 0;
  char* cur_char = shell_lastmessage;
  while (draw_y > 0) {
    if (*cur_char == '\0' || draw_x > sys_windowres_x) {
      draw_y -= shell_fontsize;
      draw_x = 0;
      cur_char--;
      continue;
    }
    DrawTextCodepoint(sys_font, *cur_char, (Vector2){ draw_x, draw_y }, shell_fontsize, WHITE);
    cur_char--;                                                                 // TODO: WARNING: YOU *WILL* FUCK EVERYTHING IF YOU DONT IMPLEMENT OVERRUN WRAPPING SOON!!!!!!!
    draw_x += shell_fontsize / 2;
  }
}

void Sys_RedrawCanvas(void) {
  BeginTextureMode(sys_canvas);
    ClearBackground(BLANK);
    Shell_Draw();
  EndTextureMode();
  sys_redraw_flag = false;
}

void Sys_Process(void) {
  if (IsKeyPressed(key_exit) && IsKeyDown(key_alt)) sys_run_flag = false;
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

  shell_buffer = calloc(1 << 20, sizeof(char));
  Shell_Print("Hello World!", sizeof("Hello World!"));
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