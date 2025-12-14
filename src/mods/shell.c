#include "shell.h"
#include "src/main.h"
#include <raylib.h>

inline char* WrapRingBuffer(char* base, char* prev, size_tits size) {
  return (prev + 1 > base + size) ? base : prev + 1;
}

char* GetNextLine(struct Shell* target, char* src) {
  char* cur_char = src;
  while (*cur_char != '\0') {
    cur_char = WrapRingBuffer(target->buffer, cur_char, SYS_EVILBUFFERSIZE);
  }
  return WrapRingBuffer(target->buffer, cur_char, SYS_EVILBUFFERSIZE);
}

void Prompt(struct Shell* target) {
  Shell_Print(target, DEF_SHELLINPUTPLACEHOLDER, sizeof(DEF_SHELLINPUTPLACEHOLDER), true);
  target->caret = target->lastmessage;
}

void InsertChar(struct Shell* target, const char insert) {
  char* cur_char = target->caret;
  while (*cur_char != '\0') {
    cur_char = WrapRingBuffer(target->buffer, cur_char, SYS_EVILBUFFERSIZE);
  }
  *WrapRingBuffer(target->buffer, cur_char, SYS_EVILBUFFERSIZE) = '\0';
  while (cur_char > target->caret) {
    *cur_char = *(cur_char - 1);
    cur_char--;
  }
  *target->caret = insert;
  target->caret = WrapRingBuffer(target->buffer, target->caret, SYS_EVILBUFFERSIZE);
  sys_redraw_flag = true;
}

void RemoveChar(struct Shell* target) {
  if (target->caret <= target->lastmessage) return;
  char* cur_char = target->caret;
  while (*cur_char != '\0') {
    cur_char = WrapRingBuffer(target->buffer, cur_char, SYS_EVILBUFFERSIZE);
  }
  while (cur_char > target->caret) {
    char last_char = *(cur_char - 1);
    *(cur_char - 1) = *cur_char;
    cur_char--;
    *(cur_char - 1) = last_char;
  }
  if (target->caret > target->lastmessage) target->caret--;
  sys_redraw_flag = true;
}

struct Shell Shell_Construct(void) {
  struct Shell shell = (struct Shell) {
    .buffer = malloc(SYS_EVILBUFFERSIZE * sizeof(char)),
    .fontsize = DEF_FONTSIZE,
    .spacing = DEF_SPACING,
  };
  if (!shell.buffer) {
    Sys_Abort(ERR_MEMALLOC);
    return (struct Shell){ 0 };
  }
  memset(shell.buffer, '\0', SYS_EVILBUFFERSIZE);
  shell.lastmessage = shell.buffer;
  shell.focusedmessage = shell.buffer;
  shell.caret = nullptr;
  return shell;
}

void Shell_OnInput(struct Shell* target) {
  char char_pressed = GetCharPressed();
  if (IsKeyPressed(KEY_ENTER)) {
    Shell_Print(target, target->lastmessage, strlen(target->lastmessage), false);
    Prompt(target);
  } else if (IsKeyPressed(KEY_BACKSPACE)) {
    RemoveChar(target);
  } else if (char_pressed) {
    InsertChar(target, char_pressed);
  }
}

void Shell_Print(struct Shell* target, const char* message, size_tits len, bool editable) {
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
  if (editable) {
    target->caret = target->lastmessage;
  } else {
    target->caret = nullptr;
  }
  sys_redraw_flag = true;
}

void Shell_Draw(struct Shell* target) {
  uint16_t draw_x = 0;
  uint16_t draw_y = 0;
  char* cur_char = target->focusedmessage;
  while (draw_y < sys_windowres_y - target->fontsize) {
    if (cur_char == target->caret) {
      DrawRectangle(draw_x, draw_y, target->fontsize / 2, target->fontsize, WHITE);
    }
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