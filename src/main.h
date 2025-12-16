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
#define DEF_PREALLOCSIZE 256
#define DEF_CACHETIMEOUT 10
#define DEF_BACKDROPCOLOR (Color){ 29, 32, 33, 255 }
#define ERR_MEMALLOC "FATAL: Memalloc failed, terminating..."
#define ERR_INDEXOOB "ERROR: Invalid array acces, out of range"
#define ERR_INVALIDOPCODE "ERROR: Supplied invalid opcode: %d"

enum OpCodes:uint8_t {
  OP_PROCESSCALL,
  OP_DRAWCALL
};

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

struct Op_ProcessCall {
  Method method;
  void* args;
} __attribute__((packed));
struct Op_DrawCall {
  Method method;
  void* args;
  uint16_t index;
} __attribute__((packed));

struct Runtime {
  uint8_t* q;
  uint16_t len;
  uint16_t cap;
  uint8_t cache_timeout;
};

struct WindowManager {
  uint16_t active_window;
  uint16_t next_id;
};

extern bool sys_run_flag;
extern RenderTexture2D sys_canvas;
extern uint16_t sys_windowres_x;
extern uint16_t sys_windowres_y;
extern Font sys_font;

extern int32_t key_exit;
extern int32_t key_alt;

extern struct Shell Sys_Shell;

uint16_t WM_CreateWindow(void);
void Sys_Abort(void);
void Sys_QueueFlush(bool clear_cache);
void Sys_Invoke(const uint8_t opcode, void* args);
void Sys_RedrawCanvas(void);
void Sys_Process(void);
void Sys_Draw(void);
void Sys_Init(void);
void Sys_Exit(void);

#endif