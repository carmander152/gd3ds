#pragma once
#include <citro2d.h>
#include "color_channels.h"

#define RGBA8(r, g, b, a) (a | (b << (u32)8) | (g << (u32)16) | (r << (u32)24))

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define CLAMP(x, lower, upper) (MIN((upper), MAX((x), (lower))))

float clampf(float d, float min, float max);
float positive_fmodf(float n, float divisor);
Color color_lerp(Color color1, Color color2, float fraction);
float map_range(float val, float min1, float max1, float min2, float max2);
float ease_out(float current, float target, float smoothing);