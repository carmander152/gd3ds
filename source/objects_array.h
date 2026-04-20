#pragma once
#include <citro2d.h>

typedef struct {
    int count;
    int* random;
    int* id;
    float* x;
    float* y;
    float* rotation;
    int* zlayer;
    int* zorder;
    float* trig_duration;
    float* width;
    float* height;
    unsigned short* v1p9_col_channel;
    unsigned short* col_channel;
    unsigned short* detail_col_channel;
    unsigned short* target_color_id;
    unsigned short* hitbox_counter;
    unsigned char* transition_applied;
    unsigned char* trig_colorR;
    unsigned char* trig_colorG;
    unsigned char* trig_colorB;
    unsigned char* orientation;
    bool* tintGround;
    bool* p1_color;
    bool* p2_color;
    bool* blending;
    bool* touch_triggered;
    bool* flippedH;
    bool* flippedV;
    bool* toggled;
    u8* activated;
    u8* collided;

    // 2.0 Movement Support
    int* group_id; 
} ObjectsArray;

extern ObjectsArray objects;
