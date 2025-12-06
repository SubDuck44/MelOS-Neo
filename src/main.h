#ifndef MELOS_MAIN
#define MELOS_MAIN
#pragma once
#include <stdint.h>
#include <stdlib.h>
#include "raylib.h"

typedef struct {
  void(*process)(uint16_t index, const void* data);
  void(*draw)(uint16_t index, const void* data, RenderTexture2D canvas);
  void(*kill)(uint16_t index, const void* data);
  const void* data;
  RenderTexture2D canvas;
  uint16_t res_x;
  uint16_t res_y;
} Window;

extern RenderTexture2D Sys_Canvas;
extern bool Sys_Run;
extern uint16_t Sys_WindowresX;
extern uint16_t Sys_WindowresY;

extern int32_t Input_Exit;
extern int32_t Input_Alt;

void sys_init(void);
void sys_exit(void);
void sys_redrawCanvas(void);
void sys_process(void);
void sys_draw(void);
uint16_t sys_addWindow(Window window);
void sys_windowQueueFree(uint16_t index);
void sys_switchWindow(uint16_t target_index);

#endif