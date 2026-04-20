#pragma once
#include <stdbool.h>
#include "level_loading.h"
#include "icons.h"
#include "particles/particles.h"
#include "trail.h"

extern int frame_skipped;
#define STEPS_HZ 240
#define STEPS_DT ((1.f + frame_skipped) / STEPS_HZ) 
#define STEPS_DT_UNMOD (1.f / STEPS_HZ) 

typedef struct {
    int slope_id;
    float elapsed;
    bool snapDown;
} SlopeData;

typedef struct {
    int object_id, player_frame, snapped_obj;
    float player_snap_diff;
} SnapData;

typedef struct { float width, height; } InternalHitbox;

typedef struct {
    int gamemode;
    float x, y, rot, scale, delta_scale, opacity, life;
    bool upside_down, active;
} P1Trail;

typedef struct {
    float x, y, vel_x, vel_y, gravity, rotation, lerp_rotation, width, height;
    InternalHitbox internal_hitbox;
    int gamemode, rotation_direction;
    bool on_ground, on_ceiling, mini, upside_down, touching_slope, inverse_rotation, snap_rotation, velocity_override;
    float robot_air_time;
    int buffering_state, frame;
    SlopeData slope_data;
    SnapData snap_data;
    P1Trail p1_trail_data[10];
} Player;

enum Gamemodes {
    GAMEMODE_PLAYER, GAMEMODE_SHIP, GAMEMODE_PLAYER_BALL,
    GAMEMODE_BIRD, GAMEMODE_DART, GAMEMODE_ROBOT
};
