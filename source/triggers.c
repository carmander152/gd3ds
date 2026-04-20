#include "triggers.h"
#include <string.h>

MoveTrigger move_triggers[MAX_ACTIVE_MOVE_TRIGGERS];

void init_triggers() {
    memset(move_triggers, 0, sizeof(move_triggers));
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

void update_triggers(float delta) {
    for (int i = 0; i < MAX_ACTIVE_MOVE_TRIGGERS; i++) {
        if (!move_triggers[i].active) continue;

        MoveTrigger *t = &move_triggers[i];
        t->elapsed += delta;
        
        float progress = t->elapsed / t->duration;
        if (progress > 1.0f) progress = 1.0f;

        // Calculate how much we need to move THIS frame
        float current_x_target = t->move_x * progress;
        float current_y_target = t->move_y * progress;
        
        float dx = current_x_target - (t->move_x * t->last_progress);
        float dy = current_y_target - (t->move_y * t->last_progress);
        
        if (t->lock_to_player_x) {
            dx = state.player.vel_x * delta;
        }

        // Apply movement to all objects in the group
        for (int j = 0; j < objects.count; j++) {
            if (objects.group_id[j] == t->target_group) {
                objects.x[j] += dx;
                objects.y[j] += dy;
                // Note: On 3DS, you may need to update the section hash 
                // if objects move too far, but for small moves, this works.
            }
        }

        t->last_progress = progress;
        if (progress >= 1.0f) t->active = false;
    }
}
