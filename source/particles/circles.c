#include "circles.h"
#include "easing.h"
#include "utils/gfx.h"
#include "state.h"

const UseEffectDefinition pad_use_effect = {
    .colorR = 1, 
    .colorG = 1,
    .colorB = 0,
    .duration = 0.25f,
    .start_opacity = 1,
    .end_opacity = 0,
    .start_rad = 0,
    .end_rad = 38,
    .hollow = false,
    .trifading = false
};

const UseEffectDefinition orb_use_effect = {
    .colorR = 1, 
    .colorG = 1,
    .colorB = 0,
    .duration = 0.4f,
    .start_opacity = 0,
    .end_opacity = 1,
    .start_rad = 35,
    .end_rad = 0,
    .hollow = false,
    .trifading = true
};

const UseEffectDefinition portal_use_effect = {
    .colorR = 1, 
    .colorG = 1,
    .colorB = 0,
    .duration = 0.4f,
    .start_opacity = 0,
    .end_opacity = 1,
    .start_rad = 40,
    .end_rad = 0,
    .hollow = false,
    .trifading = true
};

UseEffect use_effects[MAX_USE_EFFECTS];

UseEffect *add_use_effect(float x, float y, const UseEffectDefinition *def) {
    for (size_t i = 0; i < MAX_USE_EFFECTS; i++) {
        UseEffect *effect = &use_effects[i];
        if (!effect->active) {
            effect->active = true;

            effect->x = x;
            effect->y = y;
            
            // Struct copy
            effect->def = *def;

            effect->elapsed = 0;
            return effect;
        }
    }
    return NULL;
}

void update_use_effects(float delta) {
    for (size_t i = 0; i < MAX_USE_EFFECTS; i++) {
        UseEffect *effect = &use_effects[i];
        if (effect->active) {
            float opacity;
            if (effect->def.trifading) {
                if ((effect->elapsed / effect->def.duration) < 0.5f) {
                    opacity = easeValue(EASE_LINEAR, effect->def.start_opacity, effect->def.end_opacity, effect->elapsed, effect->def.duration / 2, 2.f);
                } else {
                    opacity = easeValue(EASE_LINEAR, effect->def.end_opacity, effect->def.start_opacity, effect->elapsed - (effect->def.duration / 2), effect->def.duration / 2, 2.f);
                }
            } else {
                opacity = easeValue(EASE_LINEAR, effect->def.start_opacity, effect->def.end_opacity, effect->elapsed, effect->def.duration, 2.f);
            }

            effect->opacity = get_opacity(opacity);

            effect->rad = easeValue(EASE_OUT, effect->def.start_rad, effect->def.end_rad, effect->elapsed, effect->def.duration, 2.f);

            effect->elapsed += delta;
            if (effect->elapsed >= effect->def.duration) {
                effect->active = false;
            }
        }
    }
}

void draw_use_effects() {
    for (size_t i = 0; i < MAX_USE_EFFECTS; i++) {
        UseEffect *effect = &use_effects[i];
        if (effect->active) {
            float x = effect->x;
            float y = effect->y;
            float size = effect->rad;

            float r = effect->def.colorR;
            float g = effect->def.colorG;
            float b = effect->def.colorB;
            float a = effect->opacity;

            u32 color = C2D_Color32f(r, g, b, a);

            // If stationary, dont convert to screen space
            
            x = get_mirror_x((x - state.camera_x), state.mirror_factor);
            y = GSP_SCREEN_WIDTH - ((y - state.camera_y));  

            if (effect->def.hollow) {
                custom_circunference(x, y, size, color, 2);
            } else {
                custom_circle(x, y, size, color);
            }
        }
    }
}
