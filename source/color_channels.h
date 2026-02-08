#pragma once

#include <stdbool.h>
#include "level_loading.h"
void upload_to_buffer(Object *obj, int channel);

enum ColorChannelIDs {
    NONE,
    COL_1,
    COL_2,
    COL_3,
    COL_4,
    CHANNEL_BG = 1000,
    CHANNEL_GROUND,
    CHANNEL_LINE,
    CHANNEL_3DL,
    CHANNEL_OBJ,
    CHANNEL_P1,
    CHANNEL_P2,
    CHANNEL_LBG,
    COL_CHANNEL_NUM,
};

typedef struct {
    unsigned char r,g,b;
} Color;

typedef struct {
    Color color;
    bool blending;
} ColorChannel;

typedef struct {
    bool active;
    Color old_color;
    Color new_color;
    float seconds;
    float time_run;
} ColTriggerBuffer;

extern ColorChannel channels[COL_CHANNEL_NUM];

extern Color p1_color;
extern Color p2_color;

#define BG_TRIGGER 29
#define GROUND_TRIGGER 30
#define LINE_TRIGGER 104
#define V2_0_LINE_TRIGGER 915
#define OBJ_TRIGGER 105
#define OBJ_2_TRIGGER 221
#define COL2_TRIGGER 717
#define COL3_TRIGGER 718
#define COL4_TRIGGER 743
#define THREEDL_TRIGGER 744
#define COL_TRIGGER 899

void calculate_lbg();
void init_col_channels();
void handle_col_triggers();
void handle_triggers();
void upload_to_buffer(Object *obj, int channel);
int convert_one_point_nine_channel(int channel);