#include "shell.h"

void modC_shell(void) {
  Window window = {
    .process = modP_shell,
    .draw = modD_shell,
    .kill = modK_shell,
    .data = nullptr,
    .canvas = { 0 },
    .res_x = 1280,
    .res_y = 720
  };
  sys_switchWindow(sys_addWindow(window));
  sys_redrawCanvas();
}

void modK_shell(const uint16_t index, const void* data) {
  sys_windowQueueFree(index);
}

void modD_shell(uint16_t index, const void* data, RenderTexture2D canvas) {
  BeginTextureMode(canvas);
  DrawText("test", 500, 500, 20, BLUE);
  DrawRectangle(0, 0, 1280, 720, BLUE);
  EndTextureMode();
}

void modP_shell(uint16_t index, const void* data) {
  ;
}