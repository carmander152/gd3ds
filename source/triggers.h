#pragma once
#include "state.h"

#define MAX_ACTIVE_MOVE_TRIGGERS 128

typedef struct {
    int target_group;
    float move_x;
    float move_y;
    float duration;
    float elapsed;
    float last_progress;
    bool active;
    bool lock_to_player_x;
} MoveTrigger;

extern MoveTrigger move_triggers[MAX_ACTIVE_MOVE_TRIGGERS];

void init_triggers();
void update_triggers(float delta);
void activate_move_trigger(int group, float x, float y, float duration, bool lock_x);
