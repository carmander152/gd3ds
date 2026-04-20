#include "player.h"
#include "state.h"
#include "icons.h"
#include "graphics.h"
#include "particles/particles.h"
#include "slope.h"
#include "menus/icon_kit.h"
#include "collision.h"
#include "math_helpers.h"
#include "main.h"
#include "utils/gfx.h"

inline float gravFloor(Player *player) { return player->upside_down ? -state.ceiling_y : state.ground_y; }

void robot_gamemode(Player *player) {
    trail->positionR = (Vec2){player->x, player->y};  
    trail->startingPositionInitialized = true;

    if (player->vel_y < -810) player->vel_y = -810;
    if (player->y > 2794.f) state.dead = true;

    if (player->on_ground) {
        MotionTrail_StopStroke(trail);
        update_rotation_direction(player);
        if (player->slope_data.slope_id < 0) player->rotation = 0;
    }

    // Initial Hop
    if ((player->slope_data.slope_id >= 0 || player->on_ground) && 
        (state.input.holdJump && player->buffering_state == BUFFER_READY)) {
        
        set_p_velocity(player, cube_jump_heights[state.speed] / 1.8f, false);
        player->on_ground = false;
        player->buffering_state = BUFFER_END;
        player->robot_air_time = 0.f;
        player->gravity = 0; 
        player->velocity_override = true;
    }

    // Variable Height Logic (2.0)
    if (player->robot_air_time >= 1.5f || (!state.input.holdJump)) {   
        player->gravity = cube_accelerations[state.speed] * 0.9f;
        player->velocity_override = false;
    } else if (player->buffering_state == BUFFER_END) {
        player->robot_air_time += 5.4f * STEPS_DT;
    }
}

// ... (Your other cube, ship, ball, ufo, wave gamemode functions) ...

void run_player(Player *player) {
    float scale = (player->mini) ? 0.6f : 1.f;
    trail->stroke = 10.f * scale;
    
    if (!player->left_ground) {
        if (getGroundBottom(player) <= state.ground_y) {
            player->on_ground = !player->upside_down;
            player->on_ceiling = player->upside_down;
            player->time_since_ground = 0; 
        } 
        if (getGroundTop(player) >= state.ceiling_y) {
            player->on_ground = player->upside_down;
            player->on_ceiling = !player->upside_down;
            player->time_since_ground = 0; 
        } 
    }

    switch (player->gamemode) {
        case GAMEMODE_PLAYER: cube_gamemode(player); break;
        case GAMEMODE_SHIP: ship_gamemode(player); break;
        case GAMEMODE_PLAYER_BALL: ball_gamemode(player); break;
        case GAMEMODE_BIRD: ufo_gamemode(player); break;
        case GAMEMODE_DART: wave_gamemode(player); break;
        case GAMEMODE_ROBOT: robot_gamemode(player); break;
    }
    
    player->time_since_ground += STEPS_DT;
    if (!player->velocity_override) player->vel_y += player->gravity * STEPS_DT;

    player->rotation = normalize_angle(player->rotation);
    player->lerp_rotation = iSlerp(player->lerp_rotation, player->rotation, 0.2f, STEPS_DT);

    if (getGroundBottom(player) < state.ground_y) {
        if (player->upside_down && (player->gamemode == GAMEMODE_PLAYER || player->gamemode == GAMEMODE_ROBOT)) state.dead = true;
        player->y = state.ground_y + (player->height / 2);
    }
    if (getGroundTop(player) > state.ceiling_y) {
        if (!player->upside_down && (player->gamemode == GAMEMODE_PLAYER || player->gamemode == GAMEMODE_ROBOT)) state.dead = true;
        player->y = state.ceiling_y - (player->height / 2);
    }
}

void draw_player(Player *player) {
    if (state.dead) return;
    float calc_x = ((player->x - state.camera_x));
    float calc_y = SCREEN_HEIGHT - ((player->y - state.camera_y));
    u32 primary_color = C2D_Color32(p1_color.r, p1_color.g, p1_color.b, 255);
    u32 secondary_color = C2D_Color32(p2_color.r, p2_color.g, p2_color.b, 255);
    float scale = (player->mini) ? 0.6f : 1.f;
    float p_rot = player->lerp_rotation * state.mirror_mult;
    float calc_x_mirror = get_mirror_x(calc_x, state.mirror_factor);

    switch (player->gamemode) {
        case GAMEMODE_ROBOT:
        case GAMEMODE_PLAYER:
            spawn_icon_at(GAMEMODE_PLAYER, selected_cube, player_glow_enabled, calc_x_mirror, calc_y, p_rot, (state.mirror_mult < 0), false, scale, primary_color, secondary_color, C2D_Color32(glow_color.r, glow_color.g, glow_color.b, 255));
            break;
        // ... (other cases) ...
    }
}
