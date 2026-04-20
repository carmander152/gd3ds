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
    int object_id;
    int player_frame;
    float player_snap_diff;
    int snapped_obj;
} SnapData;

typedef struct {
    float width;
    float height;
} InternalHitbox;

typedef struct {
    int gamemode;
    float x, y, rot, scale, delta_scale, opacity, life;
    bool upside_down, active;
} P1Trail;

typedef struct {
    float x, y, vel_x, vel_y, new_vel_y, delta_y, gravity, rotation, lerp_rotation, width, height;
    InternalHitbox internal_hitbox;
    int gamemode;
    int rotation_direction;
    bool on_ground, on_ceiling, mini, upside_down, touching_slope, inverse_rotation, snap_rotation;
    int potentialSlope_id;
    bool left_ground;
    float ball_rotation_speed, cutscene_timer;
    int buffering_state;
    float time_since_ground, ufo_last_y, ceiling_inv_time, timeElapsed;
    int gravObj_id;
    float burst_particle_timer, cutscene_initial_player_x, cutscene_initial_player_y;
    int slope_slide_coyote_time, frame;
    SlopeData coyote_slope, slope_data;
    SnapData snap_data;
    bool velocity_override;
    float coyote_frames;
    int p1_trail_pos;
    P1Trail p1_trail_data[10];
    float robot_air_time; // 2.0 variable jump
} Player;

enum BufferingState {
    BUFFER_NONE,
    BUFFER_READY,
    BUFFER_END
};

enum PlayerSpeeds {
    SPEED_SLOW,
    SPEED_NORMAL,
    SPEED_FAST,
    SPEED_FASTER,
    SPEED_COUNT
};

extern const float player_speeds[SPEED_COUNT];
extern const float cube_jump_heights[SPEED_COUNT];
extern const float cube_accelerations[];

inline float getTop(Player *player)  { return player->y + player->height / 2; }
inline float getBottom(Player *player)  { return player->y - player->height / 2; }
inline float getGroundTop(Player *player)  { return player->y + (player->height / 2) + ((player->gamemode == GAMEMODE_DART) ? (player->mini ? 3 : 5) : 0); }
inline float getGroundBottom(Player *player)  { return player->y - (player->height / 2) - ((player->gamemode == GAMEMODE_DART) ? (player->mini ? 3 : 5) : 0); }
inline float gravBottom(Player *player) { return player->upside_down ? -getTop(player) : getBottom(player); }
inline float grav(Player *player, float val) { return player->upside_down ? -val : val; }

void handle_player(Player *player);
void draw_player(Player *player);
void run_player(Player *player);
void update_rotation_direction(Player *player);
