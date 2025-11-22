#include "raylib.h"
#include <stdlib.h>
#define MELGRAY (Color){53, 53, 53, 255}
#define DEBUG
#define DEFAULTRES_X 1280.0f
#define DEFAULTRES_Y 720.0f
#define MAXWINDOWS 5
#define MAXTERMINALBUFFER 128

enum Errors {
  ERROR_MEMALLOC,
  ERROR_INVALIDCLASSTYPE
};
enum Classes {
  CLASS_SHELL
}; // Yes, im using classes in C. What about it?

typedef void(*Method)(void *data, int index);
typedef struct {
  void *data;
  enum Classes type;
} Instance;
typedef struct {
  Instance *queue;
  int length;
  int capacity;
} CallbackQueue;
typedef struct {
  RenderTexture2D canvas;
  Vector2 size; // Display size (how large the windows is)
  Vector2 resolution; // Display resolution (how many pixels are rendered and projected onto the size)
} Canvas;
typedef struct {
  Rectangle area;
  int process_index;
} Window;

struct Runtime {
  Canvas super_canvas;
  CallbackQueue processes;
  bool should_program_run;
  bool queue_redraw;
  short render_speed; // How many times a second the super_canvas should be drawn to SCREEN, NOT REDRAW ITS CHILDREN
} Runtime = {
  .should_program_run = true,
  .queue_redraw = true,
  .render_speed = 30
};

struct WindowManager {
  Window *windows;
  int length;
} WindowManager = {
  .windows = nullptr,
  .length = 0
};

void ThrowError(const enum Errors type) {
#ifdef DEBUG
  switch (type) {
    case ERROR_MEMALLOC:
      TraceLog(LOG_FATAL, "Failed to allocate memory");
      Runtime.should_program_run = false;
      break;
    case ERROR_INVALIDCLASSTYPE:
      TraceLog(LOG_ERROR, "Invalid class type");
      break;
    default:
  }
#else
  ;
#endif
}

int QueueFree(CallbackQueue *target, int index) {
  Instance *instance = &Runtime.processes.queue[index];
  if (instance->data != nullptr) free(instance->data);
  if (index != target->length - 1) {
    target->queue[index] = target->queue[target->length - 1];
  }
  target->length--;
  if (target->length <= target->capacity - 5) {
    Instance* temp = realloc(target->queue, sizeof(Instance) * (target->capacity - 5));
    if (!temp) {
      ThrowError(ERROR_MEMALLOC);
      return -1;
    }
    target->queue = temp;
    target->capacity -= 5;
  }
  return target->length;
}
void OnInit(void) {
  Runtime.super_canvas = (Canvas){
    .canvas = {0},
    .size = (Vector2){DEFAULTRES_X, DEFAULTRES_Y},
    .resolution = (Vector2){DEFAULTRES_X, DEFAULTRES_Y}
  };
  Runtime.processes = (CallbackQueue){
    .queue = malloc(sizeof(Instance) * 5),
    .capacity = 5,
    .length = 0
  };
  WindowManager.windows = malloc(sizeof(Window) * MAXWINDOWS);
  SetConfigFlags(FLAG_MSAA_4X_HINT);
  SetConfigFlags(FLAG_WINDOW_UNDECORATED);
  InitWindow((int)Runtime.super_canvas.size.x, (int)Runtime.super_canvas.size.y, "MelOS Neo");
  SetTargetFPS(Runtime.render_speed);
  LoadRenderTexture((int)Runtime.super_canvas.resolution.x, (int)Runtime.super_canvas.resolution.y);
}
// Runs all process callbacks
void OnProcess(void) {
  if (IsKeyPressed(KEY_ESCAPE) && IsKeyDown(KEY_LEFT_SHIFT)) Runtime.should_program_run = false;
  for (int index = 0; index < Runtime.processes.length; index++) {
    switch (Runtime.processes.queue[index].type) {
      default:
        ThrowError(ERROR_INVALIDCLASSTYPE);
        QueueFree(&Runtime.processes, index);
    }
  }
}
// Draws all callbacks linked to a certain canvas
void RedrawCanvas(void) {
  BeginTextureMode(Runtime.super_canvas.canvas);
  ClearBackground(MELGRAY);
  for (int index = 0; index < Runtime.processes.length; index++) {
    switch (Runtime.processes.queue[index].type) {
      default:
        ThrowError(ERROR_INVALIDCLASSTYPE);
        QueueFree(&Runtime.processes, index);
    }
  }
  EndTextureMode();
}
// Close the program
void OnExit(void) {
  UnloadRenderTexture(Runtime.super_canvas.canvas);
  CloseWindow();
}
int Instantiate(CallbackQueue *target, enum Classes type, void *data) {
  if (target->capacity < target->length + 1) {
    Instance *temp = realloc(target->queue, sizeof(Instance) * (target->capacity + 5));
    if (!temp) {
      ThrowError(ERROR_MEMALLOC);
      return -1;
    }
    target->queue = temp;
    target->capacity += 5;
  }
  target->queue[target->length] = (Instance){data, type};
  target->length++;
  return target->length - 1;
}
int main(void) {
  OnInit();
  while (Runtime.should_program_run) {
    OnProcess();
    if (Runtime.queue_redraw) RedrawCanvas(); // Run all draw callbacks linked to super_canvas
    BeginDrawing();
      // Draws the super_canvas onto screen each frame
      DrawTexturePro(
        Runtime.super_canvas.canvas.texture,
        (Rectangle){0, 0, Runtime.super_canvas.resolution.x, -Runtime.super_canvas.resolution.y},
        (Rectangle){0, 0, Runtime.super_canvas.size.x, Runtime.super_canvas.size.y},
        (Vector2){0.0f, 0.0f},
        0.0f, WHITE
      );
    EndDrawing();
  }
  OnExit();
  return 0;
}