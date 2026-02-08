#pragma once
#include "color_channels.h"

float clampf(float d, float min, float max);
Color color_lerp(Color color1, Color color2, float fraction);