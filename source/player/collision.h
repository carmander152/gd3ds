#pragma once

#include "player.h"

typedef struct {
    float x, y;
} Vec2D;

#define MAX_COLLIDED_OBJECTS 256

void collide_with_objects(Player *player);