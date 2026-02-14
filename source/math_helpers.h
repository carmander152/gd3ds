#pragma once
#include "color_channels.h"

#define RGBA8(r, g, b, a) (a | (b << (u32)8) | (g << (u32)16) | (r << (u32)24))

float clampf(float d, float min, float max);
float positive_fmodf(float n, float divisor);
Color color_lerp(Color color1, Color color2, float fraction);
