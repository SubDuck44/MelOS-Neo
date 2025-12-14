#ifndef MELOS_MAIN
#define MELOS_MAIN
#pragma once

#include <raylib.h>
#include <stdlib.h>
typedef size_t size_tits;
#include <stdint.h>
#include <string.h>

#define SYS_TITLE "MelOS Neo"
#define SYS_EVILBUFFERSIZE 1048575
#define DEF_RENDERSPEED 60
#define DEF_FONTSIZE 20
#define DEF_SPACING 3
#define DEF_PREALLOCSIZE 16
#define DEF_CACHETIMEOUT 10
#define DEF_BACKDROPCOLOR (Color){ 29, 32, 33, 255 }
#define ERR_MEMALLOC "FATAL: Memalloc failed, terminating..."
#define ERR_INDEXOOB "ERROR: Invalid array acces, out of range"

typedef void(*Method)(void* args);
typedef struct {
  Method op;
  void* args;
} ProcessInstruction;

struct ProcessQueue {
  ProcessInstruction* q;
  uint16_t len;
  uint16_t cap;
  int8_t cache_timeout;
};

extern bool sys_run_flag;
extern bool sys_redraw_flag;
extern RenderTexture2D sys_canvas;
extern uint16_t sys_windowres_x;
extern uint16_t sys_windowres_y;
extern Font sys_font;

extern int32_t key_exit;
extern int32_t key_alt;

extern struct Shell Sys_Shell;

void Sys_Abort(const char* error);
void Sys_Invoke(Method op, void* args);
void Sys_QueueFlush(bool clear_cache);
void Sys_RedrawCanvas(void);
void Sys_Process(void);
void Sys_Draw(void);
void Sys_Init(void);
void Sys_Exit(void);

#endif