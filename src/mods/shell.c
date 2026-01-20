#include "shell.h"
#include "src/main.h"
#include <raylib.h>

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
  SHELL_DRAW;
}

void RemoveChar(struct Shell* target) {
	uint16_t msg_ptr = target->caret;
	while (target->buffer[msg_ptr] != '\0') {
		target->buffer[msg_ptr] = target->buffer[SHELL_BUFWRAP(msg_ptr, SYS_EVILBUFFERSIZE)];
		msg_ptr = SHELL_BUFWRAP(msg_ptr, SYS_EVILBUFFERSIZE);
  }
	if (target->caret != target->lastmessage) target->caret = SHELL_BUFWRAP(
			target->caret,
			SYS_EVILBUFFERSIZE
		);
  SHELL_DRAW;
}

struct Shell Shell_Construct(void) {
  struct Shell shell = (struct Shell) {
    .buffer = calloc(SYS_EVILBUFFERSIZE, 1),
    .fontsize = DEF_FONTSIZE,
    .spacing = DEF_SPACING,
    .window_index = WM_CreateWindow(),
		.lastmessage = 0,
		.caret = 0
  };
  if (!shell.buffer) {
    TraceLog(LOG_FATAL, ERR_MEMALLOC);
    Sys_Abort();
    return (struct Shell){ 0 };
  }
  return shell;
}

void Shell_OnInput(struct Shell* target) {
  char char_pressed = GetCharPressed();
  if (IsKeyPressed(KEY_ENTER)) {
    Shell_Print(target, &target->buffer[target->lastmessage]);
    Prompt(target);
  } else if (IsKeyPressed(KEY_BACKSPACE)) {
    RemoveChar(target);
  } else if (char_pressed) {
    InsertChar(target, char_pressed);
  }
}

void Shell_Print(struct Shell* target, const char* message) {
  uint16_t buf_ptr = target->lastmessage;
	for (uint16_t i = 0; message[i] != '\0'; i++) {
		target->buffer[buf_ptr] = message[i];
		buf_ptr = SHELL_BUFWRAP(buf_ptr, SYS_EVILBUFFERSIZE);
	}
	buf_ptr = SHELL_BUFWRAP(buf_ptr, SYS_EVILBUFFERSIZE);
	target->buffer[buf_ptr] = '\0';
	target->caret = target->lastmessage;
  SHELL_DRAW;
}

void Shell_Draw(struct Shell* target) {
  uint16_t draw_x = 0;
  uint16_t draw_y = 0;
  uint16_t cur_char = target->lastmessage;
  while (draw_y > 0) {
    if (cur_char == target->caret) {
      DrawRectangle(draw_x, draw_y, target->fontsize / 2, target->fontsize, WHITE);
    }
    if (target->buffer[cur_char] == '\0' || draw_x > sys_windowres_x) {
      draw_y -= target->fontsize + target->spacing;
      draw_x = 0;
    } else {
        DrawTextCodepoint(
        sys_font, target->buffer[cur_char], (Vector2){ draw_x, draw_y }, target->fontsize, WHITE
      );
      draw_x += target->fontsize / 2;
    }
= cur_char = SHELL_BUFWRAP(cur_char, SYS_EVILBUFFERSIZE);
  }
}
