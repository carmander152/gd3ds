#pragma once
#include "state.h"
#include "objects_array.h"

#define MAX_ACTIVE_MOVE_TRIGGERS 128
#define MAX_ACTIVE_ALPHA_TRIGGERS 128

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

typedef struct {
    int target_group;
    float target_opacity;
    float start_opacity;
    float duration;
    float elapsed;
    bool active;
} AlphaTrigger;

extern MoveTrigger move_triggers[MAX_ACTIVE_MOVE_TRIGGERS];
extern AlphaTrigger alpha_triggers[MAX_ACTIVE_ALPHA_TRIGGERS];

void init_triggers();
void update_move_triggers(float delta);
void update_alpha_triggers(float delta);
void activate_move_trigger(int group, float x, float y, float duration, bool lock_x);
void activate_alpha_trigger(int group, float opacity, float duration);
