#include "triggers.h"
#include <string.h>
#include "objects_array.h"

MoveTrigger move_triggers[MAX_ACTIVE_MOVE_TRIGGERS];
AlphaTrigger alpha_triggers[MAX_ACTIVE_ALPHA_TRIGGERS];

void init_triggers() {
    memset(move_triggers, 0, sizeof(move_triggers));
    memset(alpha_triggers, 0, sizeof(alpha_triggers));
}

void activate_move_trigger(int group, float x, float y, float duration, bool lock_x) {
    for (int i = 0; i < MAX_ACTIVE_MOVE_TRIGGERS; i++) {
        if (!move_triggers[i].active) {
            move_triggers[i].target_group = group;
            move_triggers[i].move_x = x;
            move_triggers[i].move_y = y;
            move_triggers[i].duration = (duration < 0.01f) ? 0.01f : duration;
            move_triggers[i].elapsed = 0;
            move_triggers[i].last_progress = 0;
            move_triggers[i].lock_to_player_x = lock_x;
            move_triggers[i].active = true;
            return;
        }
    }
}

void activate_alpha_trigger(int group, float opacity, float duration) {
    for (int i = 0; i < MAX_ACTIVE_ALPHA_TRIGGERS; i++) {
        if (!alpha_triggers[i].active) {
            alpha_triggers[i].target_group = group;
            alpha_triggers[i].target_opacity = opacity;
            alpha_triggers[i].duration = (duration < 0.01f) ? 0.01f : duration;
            alpha_triggers[i].elapsed = 0;
            
            alpha_triggers[i].start_opacity = 1.0f; 
            for(int j=0; j < objects.count; j++) {
                if(objects.group_id[j] == group) {
                    alpha_triggers[i].start_opacity = objects.opacity[j];
                    break;
                }
            }
            alpha_triggers[i].active = true;
            return;
        }
    }
}

void update_move_triggers(float delta) {
    for (int i = 0; i < MAX_ACTIVE_MOVE_TRIGGERS; i++) {
        if (!move_triggers[i].active) continue;
        MoveTrigger *t = &move_triggers[i];
        t->elapsed += delta;
        float progress = t->elapsed / t->duration;
        if (progress > 1.0f) progress = 1.0f;

        float dx = (t->move_x * progress) - (t->move_x * t->last_progress);
        float dy = (t->move_y * progress) - (t->move_y * t->last_progress);
        if (t->lock_to_player_x) dx = state.player.vel_x * delta;

        for (int j = 0; j < objects.count; j++) {
            if (objects.group_id[j] == t->target_group) {
                objects.x[j] += dx;
                objects.y[j] += dy;
            }
        }
        t->last_progress = progress;
        if (progress >= 1.0f) t->active = false;
    }
}

void update_alpha_triggers(float delta) {
    for (int i = 0; i < MAX_ACTIVE_ALPHA_TRIGGERS; i++) {
        if (!alpha_triggers[i].active) continue;
        AlphaTrigger *t = &alpha_triggers[i];
        t->elapsed += delta;
        float progress = t->elapsed / t->duration;
        if (progress > 1.0f) progress = 1.0f;

        float current_opacity = t->start_opacity + (t->target_opacity - t->start_opacity) * progress;
        for (int j = 0; j < objects.count; j++) {
            if (objects.group_id[j] == t->target_group) objects.opacity[j] = current_opacity;
        }
        if (progress >= 1.0f) t->active = false;
    }
}
