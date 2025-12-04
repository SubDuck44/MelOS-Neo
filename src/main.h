#ifndef TERMINAL_MAIN_H
#define TERMINAL_MAIN_H

#include "raylib.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#pragma once

// Enums
enum ExitCodes {
  ERR = -1,
  OK = 0
};
enum ErrorCodes {
  ERR_MEMALLOC,
  ERR_INDEX_OOB,
  ERR_INVALIDSIZE
};

// Types
typedef struct {
  uint32_t X;
  uint32_t Y;
} Vector2i;
typedef void(*Method)(uint16_t Index, void* Data);
typedef void(*DrawCall)(RenderTexture2D Canvas, uint16_t Index, void* Data);
typedef struct {
  Method Update;
  DrawCall Draw;
  void* Data;
  RenderTexture2D Canvas;
} Sys_Window;
typedef struct {
  Sys_Window* Arr;
  uint16_t Len;
  uint16_t Cap;
} Sys_WindowQueue;

// Global variables
extern bool Sys_Run;
extern RenderTexture2D Sys_Canvas;
extern Vector2i Sys_WindowRes;
extern Sys_WindowQueue Sys_Windows;
extern uint16_t Sys_ActiveWindow;
extern Font Sys_Font;

// Global keymap
extern int32_t Key_Exit;
extern int32_t Key_Alt ;

// Sys functions
void sys_switchwindow(uint16_t Target);
void sys_redrawcanvas(void);
void sys_window_append(Sys_Window Target);
void sys_window_pop(uint16_t Index);

#endif //TERMINAL_MAIN_H