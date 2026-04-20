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
        case 10: return GD_VAL_FLOAT;
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

bool init_arrays(int count) {
    objects.id = malloc(sizeof(int) * count);
    objects.x = malloc(sizeof(float) * count);
    objects.y = malloc(sizeof(float) * count);
    objects.rotation = malloc(sizeof(float) * count);
    objects.trig_duration = malloc(sizeof(float) * count);
    objects.group_id = malloc(sizeof(int) * count); // ADDED
    objects.target_color_id = malloc(sizeof(unsigned short) * count);
    objects.trig_colorR = malloc(sizeof(unsigned char) * count);
    objects.trig_colorG = malloc(sizeof(unsigned char) * count);
    objects.activated = malloc(sizeof(u8) * count);
    // ... initialize others from your original ...
    memset(objects.activated, 0, sizeof(u8) * count);
    memset(objects.group_id, 0, sizeof(int) * count);
    return true;
}

// ... Keep your decompress_level and split_string functions ...

bool parse_string(const char *levelString) {
    int sectionCount = 0;
    char **sections = split_string(levelString, ';', &sectionCount);
    int objectCount = sectionCount - 1;
    init_arrays(objectCount);
    objects.count = objectCount;

    for (int i = 0; i < objectCount; i++) {
        int tokenCount = 0;
        char **tokens = split_string(sections[i + 1], ',', &tokenCount);
        for (int j = 0; j + 1 < tokenCount; j += 2) {
            int key = atoi(tokens[j]);
            GDValueType type = get_value_type_for_key(key);
            GDValue val;
            if (type == GD_VAL_INT) val.i = atoi(tokens[j+1]);
            else if (type == GD_VAL_FLOAT) val.f = atof(tokens[j+1]);
            else val.b = (tokens[j+1][0] == '1');
            fill_object_data(i, key, type, val);
        }
    }
    return true;
}
