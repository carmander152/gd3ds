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
        case 6:  return GD_VAL_FLOAT;
        case 10: return GD_VAL_FLOAT;
        case 11: return GD_VAL_BOOL;
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
        case 51: if (type == GD_VAL_INT) objects.target_color_id[object] = val.i; break;
        case 57: if (type == GD_VAL_INT) objects.group_id[object] = val.i; break;
        case 28: if (type == GD_VAL_FLOAT) objects.trig_colorR[object] = (unsigned char)val.f; break;
        case 29: if (type == GD_VAL_FLOAT) objects.trig_colorG[object] = (unsigned char)val.f; break;
    }
}

bool is_valid_object(int id) {
    return id >= 1 && id < 1000; // Expanded for 2.0 triggers
}

bool init_arrays(int count) {
    objects.random = malloc(sizeof(int) * count);
    objects.id = malloc(sizeof(int) * count);
    objects.x = malloc(sizeof(float) * count);
    objects.y = malloc(sizeof(float) * count);
    objects.rotation = malloc(sizeof(float) * count);
    objects.zlayer = malloc(sizeof(int) * count);
    objects.zorder = malloc(sizeof(int) * count);
    objects.trig_duration = malloc(sizeof(float) * count);
    objects.width = malloc(sizeof(float) * count);
    objects.height = malloc(sizeof(float) * count);
    objects.v1p9_col_channel = malloc(sizeof(unsigned short) * count);
    objects.col_channel = malloc(sizeof(unsigned short) * count);
    objects.detail_col_channel = malloc(sizeof(unsigned short) * count);
    objects.target_color_id = malloc(sizeof(unsigned short) * count);
    objects.hitbox_counter = malloc(sizeof(unsigned short) * count);
    objects.transition_applied = malloc(sizeof(unsigned char) * count);
    objects.trig_colorR = malloc(sizeof(unsigned char) * count);
    objects.trig_colorG = malloc(sizeof(unsigned char) * count);
    objects.trig_colorB = malloc(sizeof(unsigned char) * count);
    objects.orientation = malloc(sizeof(unsigned char) * count);
    objects.tintGround = malloc(sizeof(bool) * count);
    objects.p1_color = malloc(sizeof(bool) * count);
    objects.p2_color = malloc(sizeof(bool) * count);
    objects.blending = malloc(sizeof(bool) * count);
    objects.touch_triggered = malloc(sizeof(bool) * count);
    objects.flippedH = malloc(sizeof(bool) * count);
    objects.flippedV = malloc(sizeof(bool) * count);
    objects.toggled = malloc(sizeof(bool) * count);
    objects.activated = malloc(sizeof(u8) * count);
    objects.collided = malloc(sizeof(u8) * count);
    objects.group_id = malloc(sizeof(int) * count);

    memset(objects.id, 0, sizeof(int) * count);
    memset(objects.activated, 0, sizeof(u8) * count);
    memset(objects.group_id, 0, sizeof(int) * count);
    return true;
}

// ... (Rest of your original decompression and string splitting logic) ...

bool parse_string(const char *levelString) {
    int sectionCount = 0;
    char **sections = split_string(levelString, ';', &sectionCount);
    if (sectionCount < 3) { free_string_array(sections, sectionCount); return false; }
    int objectCount = sectionCount - 1;
    if (!init_arrays(objectCount)) return false;
    objects.count = objectCount;
    for (int i = 0; i < objectCount; i++) {
        // Parse the object
        int tokenCount = 0;
        char **tokens = split_string(sections[i+1], ',', &tokenCount);
        for(int j=0; j+1 < tokenCount; j+=2) {
            int key = atoi(tokens[j]);
            GDValueType type = get_value_type_for_key(key);
            GDValue val;
            if(type == GD_VAL_INT) val.i = atoi(tokens[j+1]);
            else if(type == GD_VAL_FLOAT) val.f = atof(tokens[j+1]);
            else val.b = (tokens[j+1][0] == '1');
            fill_object_data(i, key, type, val);
        }
        free_string_array(tokens, tokenCount);
        assign_object_to_section(i);
    }
    free_string_array(sections, sectionCount);
    return true;
}
