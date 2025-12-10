#ifndef MELOS_MYSTRING
#define MELOS_MYSTRING
#pragma once

#include "../main.h"

void mstr_memset(char* src, size_tits size, char fill);
char* mstr_lencpy(const char* src, char* dest, size_tits size); /* Dont trust the acronym, its confusing on purpose.
If you wanna know what it means without reading the source yourself, go ask the author. Fuck you. */

#endif