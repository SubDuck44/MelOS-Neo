#include "raylib.h"
#include <stdlib.h>

const char* err_memalloc = "Heap allocation failed";

enum CallbackTypes {
    CALLBACK_EVENT,
    CALLBACK_MODULE
};

typedef void(*Function)(void);
typedef void(*Method)(unsigned, void*);

typedef struct {
    int x;
    int y;
} Vector2i;

typedef struct {
    Function call;
    void* data;
} EventCallback;

typedef struct {
    Method update;
    Method draw;
    void* data;
} ModuleCallback;

struct Sys_Eventqueue {
    EventCallback* queue;
    unsigned capacity;
    unsigned occupancy;
};

struct Sys_ModuleQueue {
    ModuleCallback* queue;
    unsigned capacity;
    unsigned occupancy;
};

struct System {
    char run;
    struct Sys_Eventqueue ev_queue;
    struct Sys_ModuleQueue mod_queue;
    char queue_redraw;
} System = {
    .run = true,
    .ev_queue = {NULL, 5, 0},
    .mod_queue = {NULL, 5, 0},
    .queue_redraw = true
};

struct RenderComponents {
    RenderTexture2D viewport;
    unsigned short render_speed;
    Vector2i resolution;
    Vector2 master_size;
    Font default_font;
    float default_font_spacing;
} RenderComponents = {
    .viewport = {0},
    .resolution = (Vector2i){1280, 720},
    .master_size = (Vector2){1280, 720},
    .render_speed = 30,
    .default_font = {0},
    .default_font_spacing = 2
};

struct Keymap {
    int modifier;
    int force_quit;
    int toggle_fullscreen;
} Keymap = {
    .modifier = KEY_LEFT_SHIFT,
    .force_quit = KEY_ESCAPE,
    .toggle_fullscreen = KEY_F11
};

/* FORWARD DECLERATIONS BEGIN */
Rectangle DrawTab(Vector2 position, Vector2 size, float bar_size, bool bar_top, const char* tab_text, Color color);
/* FORWARD DECLERATIONS END */

void Sys_Init(void) {
    SetConfigFlags(FLAG_WINDOW_TRANSPARENT);
    SetConfigFlags(FLAG_WINDOW_UNDECORATED);
    ChangeDirectory("..");
    InitWindow(RenderComponents.master_size.x, RenderComponents.master_size.y, "Terminal");
    SetTargetFPS(RenderComponents.render_speed);
    System.ev_queue.queue = malloc(sizeof(EventCallback) * 5);
    System.mod_queue.queue = malloc(sizeof(ModuleCallback) * 5);
    RenderComponents.viewport = LoadRenderTexture(RenderComponents.resolution.x, RenderComponents.resolution.y);
    RenderComponents.default_font = LoadFont("res/iosevka-regular.ttf");
}
void Sys_Terminate(void) {
    UnloadRenderTexture(RenderComponents.viewport);
    UnloadFont(RenderComponents.default_font);
    CloseWindow();
}
void Sys_Update(void) {
    int key_input = GetKeyPressed();
    if (key_input) {
        if (IsKeyDown(Keymap.modifier)) {
            if (key_input == Keymap.force_quit) System.run = false;
            if (key_input == Keymap.toggle_fullscreen) ToggleBorderlessWindowed();
        } else {
            if (key_input == Keymap.toggle_fullscreen) ToggleFullscreen();
        }
    }
    for (unsigned index = 0; index < System.ev_queue.occupancy; index++) {
        System.ev_queue.queue[index].call();
    }
    for (unsigned index = 0; index < System.mod_queue.occupancy; index++) {
        System.mod_queue.queue[index].update(index, System.mod_queue.queue[index].data);
    }
}

void Sys_Draw(void) {
    BeginTextureMode(RenderComponents.viewport);
    ClearBackground(BLANK);
        DrawTab(
            (Vector2){0.0f, 0.0f},
            (Vector2){RenderComponents.resolution.x, RenderComponents.resolution.y},
            30.0f,
            false,
            "MeLOS Neo",
            WHITE
        );
        for (unsigned index = 0; index < System.mod_queue.occupancy; index++) {
            System.mod_queue.queue[index].draw(index, System.mod_queue.queue[index].data);
        }
    EndTextureMode();
}

void Sys_Instantiate(int type, Method update, Method draw, Function call, void* data) {
    switch (type) {
        case CALLBACK_EVENT:
            if (System.ev_queue.occupancy + 1 > System.ev_queue.capacity) {
                EventCallback* temp = realloc(System.ev_queue.queue, sizeof(EventCallback) * (System.ev_queue.capacity + 5));
                if (!temp) {
                    TraceLog(LOG_FATAL, err_memalloc);
                    return;
                }
                System.ev_queue.queue = temp;
            }
            System.ev_queue.queue[System.ev_queue.occupancy] = (EventCallback){call, data};
            System.ev_queue.occupancy++;
            break;
        case CALLBACK_MODULE:
            if (System.mod_queue.occupancy + 1 > System.ev_queue.capacity) {
                ModuleCallback* temp = realloc(System.mod_queue.queue, sizeof(ModuleCallback) * (System.ev_queue.capacity + 5));
                if (!temp) {
                    TraceLog(LOG_FATAL, err_memalloc);
                    return;
                }
                System.mod_queue.queue = temp;
            }
            System.mod_queue.queue[System.mod_queue.occupancy] = (ModuleCallback){update, draw, data};
            System.mod_queue.occupancy++;
            break;
        default:
            return;
    }
}

void Sys_Kill(int type, unsigned index) {
    switch (type) {
        case CALLBACK_EVENT:
            EventCallback* current1 = &System.ev_queue.queue[index];
            if (current1->data) free(current1->data);
            if (index != System.ev_queue.occupancy - 1) System.ev_queue.queue[index] = System.ev_queue.queue[System.ev_queue.occupancy - 1];
            System.ev_queue.occupancy--;
            if (System.ev_queue.occupancy <= System.ev_queue.capacity - 5) {
                EventCallback* temp = realloc(System.ev_queue.queue, sizeof(EventCallback) * System.ev_queue.capacity - 5);
                if (!temp) {
                    TraceLog(LOG_FATAL, err_memalloc);
                    return;
                }
                System.ev_queue.queue = temp;
            }
            break;
        case CALLBACK_MODULE:
            ModuleCallback* current2 = &System.mod_queue.queue[index];
            if (current2->data) free(current2->data);
            if (index != System.mod_queue.occupancy - 1) System.mod_queue.queue[index] = System.mod_queue.queue[System.mod_queue.occupancy - 1];
            System.mod_queue.occupancy--;
            if (System.mod_queue.occupancy <= System.mod_queue.capacity - 5) {
                ModuleCallback* temp = realloc(System.mod_queue.queue, sizeof(ModuleCallback) * System.mod_queue.capacity - 5);
                if (!temp) {
                    TraceLog(LOG_FATAL, err_memalloc);
                    return;
                }
                System.mod_queue.queue = temp;
            }
            break;
        default:
            return;
    }
}
// ### MAIN ###
int main(void) {
    Sys_Init();
    while (System.run) {
        Sys_Update();
        if (System.queue_redraw) {
            Sys_Draw();
            System.queue_redraw = false;
        }
        BeginDrawing();
        ClearBackground(BLANK);
        DrawTexturePro(
            RenderComponents.viewport.texture,
            (Rectangle){0, 0, RenderComponents.resolution.x, -RenderComponents.resolution.y},
            (Rectangle){0, 0, RenderComponents.master_size.x, RenderComponents.master_size.y},
            (Vector2){0, 0},
            0.0f,
            WHITE
        );
        EndDrawing();
    }
    Sys_Terminate();
    return 0;
}
// ### MAIN ###

Rectangle DrawTab(Vector2 position, Vector2 size, float bar_size, bool bar_top, const char* tab_text, Color color) {
    float offset_y = 0.0f;
    float other_offset_y = 0.0f; // What am i even doing anymore
    DrawRectangleLinesEx((Rectangle){position.x, position.y, size.x, size.y}, 5, color);
    if (!bar_top) {
        offset_y = size.y - bar_size;
        other_offset_y = size.y - (bar_size * 2);
    }
    DrawLineEx(
        (Vector2){position.x, position.y + bar_size + other_offset_y},
        (Vector2){position.x + size.x, position.y + bar_size + other_offset_y},
        5.0f,
        color
    );
    DrawTextEx(
        RenderComponents.default_font,
        tab_text,
        (Vector2){position.x + 8, position.y + 3 + offset_y},
        bar_size - 6,
        RenderComponents.default_font_spacing,
        color
    );
    return (Rectangle){position.x + 5, position.y + 7 + bar_size, size.x - 10, size.y - 7 - bar_size};
}

// struct Mod_Terminal {
//     Vector2 size;
//     Vector2 position;
// };
// void MU_Terminal(unsigned index, void* data);
// void MD_Terminal(unsigned index, void* data) {
//     struct Mod_Terminal* self = data;
//     DrawRectangleLines(
//         self->position.x, self->position.y,
//         self->size.x, self->size.y,
//         WHITE
//     );
// }
// void MC_Terminal(Vector2 position, Vector2 size) {
//     struct Mod_Terminal* data = malloc(sizeof(struct Mod_Terminal));
//     if (!data) {
//         TraceLog(LOG_FATAL, err_memalloc);
//         return;
//     }
//     Sys_Instantiate(CALLBACK_MODULE, MU_Terminal, MD_Terminal, NULL, data);
// }