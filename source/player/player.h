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

// Trail and Particle Constants
#define P1_TRAIL_LENGTH 10
#define P1_TRAIL_DURATION 10
#define BALL_SLOW_ROTATION 0.7f

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
    int gamemode, rotation_direction;
    bool on_ground, on_ceiling, mini, upside_down, touching_slope, inverse_rotation, snap_rotation, left_ground, velocity_override;
    int potentialSlope_id, buffering_state, gravObj_id, frame;
    float ball_rotation_speed, cutscene_timer, time_since_ground, ufo_last_y, ceiling_inv_time, timeElapsed;
    float burst_particle_timer, cutscene_initial_player_x, cutscene_initial_player_y, coyote_frames, robot_air_time;
    int slope_slide_coyote_time, p1_trail_pos;
    SlopeData coyote_slope, slope_data;
    SnapData snap_data;
    P1Trail p1_trail_data[P1_TRAIL_LENGTH];
} Player;

enum BufferingState { BUFFER_NONE, BUFFER_READY, BUFFER_END };
enum PlayerSpeeds { SPEED_SLOW, SPEED_NORMAL, SPEED_FAST, SPEED_FASTER, SPEED_COUNT };

// Externs for Trails and Particles
extern MotionTrail *trail;
extern MotionTrail trail_p1, trail_p2;
extern MotionTrail *wave_trail;
extern MotionTrail wave_trail_p1, wave_trail_p2;
extern ParticleSystem coin_pickup_particles;
extern ParticleSystem land_particles[2];

extern const float player_speeds[SPEED_COUNT];
extern const float cube_jump_heights[SPEED_COUNT];
extern const float cube_accelerations[];

// Physics Inline Helpers
inline float getTop(Player *player) { return player->y + player->height / 2; }
inline float getBottom(Player *player) { return player->y - player->height / 2; }
inline float getLeft(Player *player) { return player->x - player->width / 2; }
inline float getRight(Player *player) { return player->x + player->width / 2; }

inline float getGroundTop(Player *player) { return player->y + (player->height / 2) + ((player->gamemode == GAMEMODE_DART) ? (player->mini ? 3 : 5) : 0); }
inline float getGroundBottom(Player *player) { return player->y - (player->height / 2) - ((player->gamemode == GAMEMODE_DART) ? (player->mini ? 3 : 5) : 0); }

inline float gravBottom(Player *player) { return player->upside_down ? -getTop(player) : getBottom(player); }
inline float gravTop(Player *player) { return player->upside_down ? -getBottom(player) : getTop(player); }

inline float gravInternalBottom(Player *player) { return player->upside_down ? -(player->y + player->internal_hitbox.height / 2) : (player->y - player->internal_hitbox.height / 2); }
inline float gravInternalTop(Player *player) { return player->upside_down ? -(player->y - player->internal_hitbox.height / 2) : (player->y + player->internal_hitbox.height / 2); }

inline float grav(Player *player, float val) { return player->upside_down ? -val : val; }

inline float obj_getTop(int obj) { return objects.y[obj] + objects.height[obj] / 2; }
inline float obj_getBottom(int obj) { return objects.y[obj] - objects.height[obj] / 2; }
inline float obj_getLeft(int obj) { return objects.x[obj] - objects.width[obj] / 2; }
inline float obj_getRight(int obj) { return objects.x[obj] + objects.width[obj] / 2; }

inline float obj_gravBottom(Player *player, int obj) { return player->upside_down ? -obj_getTop(obj) : obj_getBottom(obj); }
inline float obj_gravTop(Player *player, int obj) { return player->upside_down ? -obj_getBottom(obj) : obj_getTop(obj); }

void handle_player(Player *player);
void draw_player(Player *player);
void run_player(Player *player);
void update_rotation_direction(Player *player);
void set_p_velocity(Player *player, float velocity, bool override);
