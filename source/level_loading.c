#include <stdlib.h>
#include <stdbool.h>
#include "level_loading.h"
#include "defines.h"
#include <zlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include "color_channels.h"
#include "objects.h"
#include "objects_array.h"
#include "mp3_player.h"
#include "graphics.h"
#include "math_helpers.h"
#include "state.h"

ObjectsArray objects = { 0 };

int convert_object(int id) {
    switch (id) {
        case 901: return 901; // Move Trigger
        case 1007: return 1007; // Alpha Trigger
        case 143: return 143; // Robot Portal
        case 1734: return 675;
        case 1329: return SECRET_COIN;
    }
    return id;
}

GDValueType get_value_type_for_key(int key) {
    switch (key) {
        case 1:  return GD_VAL_INT;
        case 2:  return GD_VAL_FLOAT;
        case 3:  return GD_VAL_FLOAT;
        case 10: return GD_VAL_FLOAT; // Duration
        case 35: return GD_VAL_FLOAT; // Opacity
        case 51: return GD_VAL_INT;   // Target Group
        case 57: return GD_VAL_INT;   // Group ID
        case 28: return GD_VAL_FLOAT; // Move X
        case 29: return GD_VAL_FLOAT; // Move Y
        default: return GD_VAL_INT;
    }
}

void fill_object_data(int object, int key, GDValueType type, GDValue val) {
    switch (key) {
        case 1:  if (type == GD_VAL_INT) objects.id[object] = convert_object(val.i); break;
        case 2:  if (type == GD_VAL_FLOAT) objects.x[object] = val.f; break;
        case 3:  if (type == GD_VAL_FLOAT) objects.y[object] = val.f; break;
        case 10: if (type == GD_VAL_FLOAT) objects.trig_duration[object] = val.f; break;
        case 35: if (type == GD_VAL_FLOAT) objects.opacity[object] = val.f; break;
        case 51: if (type == GD_VAL_INT) objects.target_color_id[object] = val.i; break;
        case 57: if (type == GD_VAL_INT) objects.group_id[object] = val.i; break;
        case 28: if (type == GD_VAL_FLOAT) objects.trig_colorR[object] = (unsigned char)val.f; break;
        case 29: if (type == GD_VAL_FLOAT) objects.trig_colorG[object] = (unsigned char)val.f; break;
    }
}

bool init_arrays(int count) {
    objects.id = malloc(sizeof(int) * count);
    objects.x = malloc(sizeof(float) * count);
    objects.y = malloc(sizeof(float) * count);
    objects.rotation = malloc(sizeof(float) * count);
    objects.trig_duration = malloc(sizeof(float) * count);
    objects.group_id = malloc(sizeof(int) * count);
    objects.opacity = malloc(sizeof(float) * count); 
    objects.target_color_id = malloc(sizeof(unsigned short) * count);
    objects.trig_colorR = malloc(sizeof(unsigned char) * count);
    objects.trig_colorG = malloc(sizeof(unsigned char) * count);
    objects.activated = malloc(sizeof(u8) * count);

    // Initialize defaults
    for(int i=0; i<count; i++) {
        objects.opacity[i] = 1.0f;
        objects.group_id[i] = 0;
        objects.activated[i] = 0;
    }
    return true;
}

void free_arrays() {
    free(objects.id); free(objects.x); free(objects.y);
    free(objects.rotation); free(objects.trig_duration);
    free(objects.group_id); free(objects.opacity);
    free(objects.target_color_id); free(objects.trig_colorR);
    free(objects.trig_colorG); free(objects.activated);
}

// ... (Rest of parse_string and load_level functions) ...
