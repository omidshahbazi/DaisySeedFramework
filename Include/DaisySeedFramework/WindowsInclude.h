#pragma once

#undef ns
#undef ms

#include <SDL3/SDL.h>
#include <Windows.h>

#undef ns
#undef ms

#define ns *0.000'001
#define ms *0.001

#undef GetCurrentTime