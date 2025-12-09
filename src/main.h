#ifndef MELOS_MAIN
#define MELOS_MAIN
#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "raylib.h"

#define SYS_DEF_WINDOWRESX 1280
#define SYS_DEF_WINDOWRESY 720
#define SYS_WINDOWTITLE "MelOS Neo"
#define SYS_DEF_RENDERSPEED 30
#define SYS_DEF_PREALLOCNUM 5
#define SYS_DEF_SPACING 3

#define ERR_MEMALLOC "Memory allocation failed"
#define ERR_INDEXOOB "Index out of range"

enum Ker_Textures {
  KER_TXT_EYE = 0,
  KER_TXT_EYE_CLSD = 1,
  KER_TXT_MOUTH = 2,
  KER_TXT_MOUTH_OPN = 3
};

typedef struct {
  void(*process)(uint16_t index, void* data);
  void(*draw)(uint16_t index, void* data, RenderTexture2D canvas);
  void(*kill)(uint16_t index, void* data);
  void* data;
  RenderTexture2D canvas;
  uint16_t res_x;
  uint16_t res_y;
} Window;

extern RenderTexture2D Sys_Canvas;
extern Texture2D Sys_Logo;
extern Font Sys_Font;
extern bool Sys_Run;
extern uint16_t Sys_WindowresX;
extern uint16_t Sys_WindowresY;

extern struct Sys_WindowManager Sys_WindowManager;

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