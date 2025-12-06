#ifndef MELOS_MOD_SHELL
#define MELOS_MOD_SHELL
#include "../main.h"
#pragma once

void modC_shell(void);
void modK_shell(uint16_t index, const void* data);
void modD_shell(uint16_t index, const void* data, RenderTexture2D canvas);
void modP_shell(uint16_t index, const void* data);

#endif