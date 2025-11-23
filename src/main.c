#include "raylib.h"
#include <stdlib.h>
#define MELGRAY (Color){53, 53, 53, 255}
#define DEBUG
#define DEFAULTRES_X 1280.0f
#define DEFAULTRES_Y 720.0f
#define MAXWINDOWS 5
#define MAXTERMINALBUFFER 128
#define PREALLOC_BLOCKSIZE 5

enum Signals {
  SIG_PROCESS,
  SIG_DRAW
};
enum Errors {
  ERROR_MEMALLOC,
  ERROR_INVALIDCLASSTYPE
};

typedef void(*Method)(void *data, void* arg_buf);
typedef struct {
  RenderTexture2D canvas;
  Vector2 size; // Display size (how large the windows is)
  Vector2 resolution; // Display resolution (how many pixels are rendered and projected onto the size)
} Canvas;
struct SignalSubscription {
  Method method;
  void* target;
};
typedef struct {
  struct SignalSubscription* subscribers;
  unsigned sub_count;
  unsigned sub_cap;
  void* argument_buf;
} Signal;

struct Runtime {
  struct {
    Signal* signals;
    unsigned num;
    unsigned cap;
  } Signals;
  Canvas super_canvas;
  bool should_program_run;
  bool queue_redraw;
  short render_speed; // How many times a second the super_canvas should be drawn to SCREEN, NOT REDRAW ITS CHILDREN
} Runtime = {
  .should_program_run = true,
  .queue_redraw = true,
  .render_speed = 30
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
void CreateSignal(size_t buf_size) {
  if (Runtime.Signals.num + 1 > Runtime.Signals.cap) {
    Signal* temp = realloc(Runtime.Signals.signals, sizeof(Signal) * (Runtime.Signals.cap + PREALLOC_BLOCKSIZE));
    if (!temp) {
      ThrowError(ERROR_MEMALLOC);
      return;
    }
    Runtime.Signals.signals = temp;
    Runtime.Signals.cap += PREALLOC_BLOCKSIZE;
  }
  Runtime.Signals.signals[Runtime.Signals.num] = (Signal){
    .subscribers = malloc(sizeof(struct SignalSubscription) * PREALLOC_BLOCKSIZE),
    .sub_count = 0,
    .sub_cap = 0,
    .argument_buf = malloc(buf_size)
  };
  Runtime.Signals.num++;
}
void DeleteSignal(unsigned signal) {
  if (signal != Runtime.Signals.num - 1) {
    Runtime.Signals.signals[signal] = Runtime.Signals.signals[Runtime.Signals.num - 1];
  }
  Runtime.Signals.num--;
  if (Runtime.Signals.num <= Runtime.Signals.cap - PREALLOC_BLOCKSIZE) {
    Signal* temp = realloc(Runtime.Signals.signals, sizeof(Signal) * (Runtime.Signals.cap - PREALLOC_BLOCKSIZE));
    if (!temp) {
      ThrowError(ERROR_MEMALLOC);
      return;
    }
    Runtime.Signals.signals = temp;
  }
}
void ConnectSignal(Method method, void* target, unsigned signal) {
  Signal* target_signal = &Runtime.Signals.signals[signal];
  if (target_signal->sub_count + 1 > target_signal->sub_cap) {
    struct SignalSubscription* temp = realloc(target_signal->subscribers, sizeof(struct SignalSubscription) * (target_signal->sub_cap + PREALLOC_BLOCKSIZE));
    if (!temp) {
      ThrowError(ERROR_MEMALLOC);
      return;
    }
    target_signal->subscribers = temp;
    target_signal->sub_cap += PREALLOC_BLOCKSIZE;
  }
  target_signal->subscribers[target_signal->sub_count] = (struct SignalSubscription){
    .method = method,
    .target = target
  };
  target_signal->sub_count++;
}
void DisconnectSignal(void* target, unsigned signal) {
  Signal* target_signal = &Runtime.Signals.signals[signal];
  for (unsigned index = 0; index < target_signal->sub_count; index++) {
    if (target_signal->subscribers[index].target == target) {
      if (index != target_signal->sub_count - 1) {
        target_signal->subscribers[index] = target_signal->subscribers[target_signal->sub_count - 1];
      }
      target_signal->sub_count--;
      if (target_signal->sub_count <= target_signal->sub_cap) {
        struct SignalSubscription* temp = realloc(target_signal->subscribers, sizeof(struct SignalSubscription) * (target_signal->sub_cap - PREALLOC_BLOCKSIZE));
        if (!temp) {
          ThrowError(ERROR_MEMALLOC);
          return;
        }
        target_signal->subscribers = temp;
        target_signal->sub_cap -= PREALLOC_BLOCKSIZE;
      }
    }
  }
}
void EmitSignal(unsigned signal) {
  Signal* target_signal = &Runtime.Signals.signals[signal];
  for (unsigned index = 0; index < target_signal->sub_count; index++) {
    target_signal->subscribers[index].method(target_signal->subscribers[index].target, target_signal->argument_buf);
  }
}
void OnProcess(void) {
  if (IsKeyPressed(KEY_ESCAPE) && IsKeyDown(KEY_LEFT_SHIFT)) {
    Runtime.should_program_run = false;
  }
}
void OnRedraw(void) {
  ClearBackground(MELGRAY);
}
void OnInit(void) {
  Runtime.super_canvas = (Canvas){
    .canvas = {0},
    .size = (Vector2){DEFAULTRES_X, DEFAULTRES_Y},
    .resolution = (Vector2){DEFAULTRES_X, DEFAULTRES_Y}
  };

  Runtime.Signals.signals = malloc(sizeof(Signal) * PREALLOC_BLOCKSIZE);
  Runtime.Signals.num = 0;
  Runtime.Signals.cap = 5;
  CreateSignal(sizeof(unsigned)); // SIG_PROCESS
  CreateSignal(sizeof(unsigned)); // SIG_DRAW

  SetConfigFlags(FLAG_MSAA_4X_HINT);
  SetConfigFlags(FLAG_WINDOW_UNDECORATED);
  InitWindow((int)Runtime.super_canvas.size.x, (int)Runtime.super_canvas.size.y, "MelOS Neo");
  SetTargetFPS(Runtime.render_speed);
  LoadRenderTexture((int)Runtime.super_canvas.resolution.x, (int)Runtime.super_canvas.resolution.y);

}
// Close the program
void OnExit(void) {
  UnloadRenderTexture(Runtime.super_canvas.canvas);
  CloseWindow();
}
int main(void) {
  OnInit();
  while (Runtime.should_program_run) {
    OnProcess();
    EmitSignal(SIG_PROCESS);
    if (Runtime.queue_redraw) {
      BeginTextureMode(Runtime.super_canvas.canvas);
        OnRedraw();
        EmitSignal(SIG_DRAW); // Run all draw callbacks linked to super_canvas
      EndTextureMode();
    }
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