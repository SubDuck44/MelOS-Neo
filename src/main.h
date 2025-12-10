#ifndef MELOS_MAIN
#define MELOS_MAIN
#pragma once

#include "raylib.h"
#include <stdlib.h>
typedef size_t size_tits;
#include <stdint.h>
#include "mods/mystring.h"

#define SYS_TITLE "MelOS Neo"
#define DEF_RENDERSPEED 60

extern bool sys_run_flag;
extern bool sys_redraw_flag;
extern RenderTexture2D sys_canvas;
extern uint16_t sys_windowres_x;
extern uint16_t sys_windowres_y;

extern int32_t key_exit;
extern int32_t key_alt;

void Sys_RedrawCanvas(void);
void Sys_Process(void);
void Sys_Draw(void);
void Sys_Init(void);
void Sys_Exit(void);

#endif