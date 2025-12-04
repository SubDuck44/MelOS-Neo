#include "shell.h"
#include "../main.h"

#define TERMINAL_SHELL_DEFAULT_SPACING 3

void modC_shell(void) {
  struct mod_shell* data = malloc(sizeof(struct mod_shell));
  if (!data) {
    TraceLog(LOG_FATAL, "%d", ERR_MEMALLOC);
    return;
  }
  Sys_Window self = (Sys_Window){
    modU_shell,
    modD_shell,
    data,
    { 0 }
  };
  sys_window_append(self);
}

void modK_shell(uint16_t Index, struct mod_shell* data) {
  free(data);
  sys_window_pop(Index);
}

inline void modU_shell(uint16_t Index, void* Data) {
  ;
}

void modD_shell(RenderTexture2D Canvas, uint16_t Index, void* Data) {
  struct mod_shell* self = Data;
  float codepoint_width = MeasureTextEx(Sys_Font, "a", self->FontSize, TERMINAL_SHELL_DEFAULT_SPACING).x;
  float max_width = Canvas.texture.width - (Canvas.texture.width % (int)codepoint_width);
  Vector2 draw_at = (Vector2){ 0, 0 };
  for (uint16_t line_index = self->VIndex; line_index <= self->NumLines; line_index++) {

  }
}