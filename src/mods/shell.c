#include "shell.h"

inline char* WrapRingBuffer(char* base, char* prev, size_tits size) {
  return (prev + 1 > base + size) ? base : prev + 1;
}

struct Shell Shell_Construct(void) {
  struct Shell shell = (struct Shell) {
    .buffer = calloc(SYS_EVILBUFFERSIZE, sizeof(char)),
    .fontsize = DEF_FONTSIZE,
    .spacing = DEF_SPACING
  };
  shell.lastmessage = shell.buffer;
  shell.focusedmessage = shell.buffer;
  return shell;
}

void Shell_Print(struct Shell* target, const char* message, size_tits len) {
  char* begin = target->lastmessage;
  while (*begin != '\0') {
    begin = WrapRingBuffer(target->buffer, begin, SYS_EVILBUFFERSIZE);
  }
  begin = WrapRingBuffer(target->buffer, begin, SYS_EVILBUFFERSIZE);
  target->lastmessage = begin;
  for (uintptr_t i = 0; i < len; i++) {
    *begin = message[i];
    begin = WrapRingBuffer(target->buffer, begin, SYS_EVILBUFFERSIZE);
  }
  *begin = '\0';
  sys_redraw_flag = true;
}

void Shell_Draw(struct Shell* target) {
  uint16_t draw_x = 0;
  uint16_t draw_y = 0;
  char* cur_char = target->focusedmessage;
  while (draw_y < sys_windowres_y - target->fontsize) {
    if (*cur_char == '\0' || draw_x > sys_windowres_x) {
      draw_y += target->fontsize + target->spacing;
      draw_x = 0;
    } else {
        DrawTextCodepoint(
        sys_font, *cur_char, (Vector2){ draw_x, draw_y }, target->fontsize, WHITE
      );
      draw_x += target->fontsize / 2;
    }
    cur_char = WrapRingBuffer(target->buffer, cur_char, SYS_EVILBUFFERSIZE);
  }
}