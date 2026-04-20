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

    // Initiation Jump (Tap)
    if ((player->slope_data.slope_id >= 0 || player->on_ground) && 
        (state.input.holdJump && player->buffering_state == BUFFER_READY)) {
        
        // Robot starts with a small hop
        set_p_velocity(player, cube_jump_heights[state.speed] / 1.8f, false);
        
        player->on_ground = false;
        player->robot_anim_timer = 0;
        player->buffering_state = BUFFER_END;
        player->robot_air_time = 0.f;
        player->gravity = 0; // Suspend gravity during active thrust
        player->velocity_override = true;
    }

    // Thrust Logic (Hold)
    if (player->robot_air_time >= 1.5f || (!state.input.holdJump)) {   
        // Release thrust or hit max time
        player->gravity = cube_accelerations[state.speed] * 0.9f;
        player->velocity_override = false;
    } else if (player->buffering_state == BUFFER_END) {
        // Continue upward thrust while holding
        player->robot_air_time += 5.4f * STEPS_DT;
    }
}

// ... Keep other gamemode functions (cube_gamemode, ship_gamemode, etc.) as they were ...

void run_player(Player *player) {
    float scale = (player->mini) ? 0.6f : 1.f;
    trail->stroke = 10.f * scale;
    
    if (!player->left_ground) {
        if (getGroundBottom(player) <= state.ground_y) {
            if (player->upside_down) { player->on_ceiling = true; player->inverse_rotation = false; } 
            else { player->on_ground = true; player->inverse_rotation = false; }
            player->time_since_ground = 0; 
        } 
        if (getGroundTop(player) >= state.ceiling_y) {
            if (player->upside_down) { player->on_ground = true; player->inverse_rotation = false; } 
            else { player->on_ceiling = true; player->inverse_rotation = false; } 
            player->time_since_ground = 0; 
        } 
    }
    
    if (player->gamemode != GAMEMODE_DART && !state.old_player.on_ground && player->on_ground) {
        land_particles[state.current_player].emitterX = player->x;
        land_particles[state.current_player].emitterY = fabsf(gravBottom(player)) + (player->upside_down ? -4 : 4);
        land_particles[state.current_player].gravityFlipped = player->upside_down;
        land_particles[state.current_player].scale = (player->mini ? 0.6f : 1.0f);
        spawnMultipleParticles(&land_particles[state.current_player], 10);
    }

    if (gravBottom(&state.old_player) > gravFloor(&state.old_player) && player->upside_down == state.old_player.upside_down && !player->on_ground && player->vel_y <= 0) {
        if (state.old_player.on_ground && !state.old_input.holdJump) player->coyote_frames = 0;
        player->coyote_frames++;
    } else { player->coyote_frames = INT32_MAX; }

    switch (player->gamemode) {
        case GAMEMODE_PLAYER: cube_gamemode(player); break;
        case GAMEMODE_SHIP:
            glitter_particles.emitterX = state.camera_x_middle;
            glitter_particles.emitterY = state.camera_y_middle;
            glitter_particles.emitting = true;
            MotionTrail_ResumeStroke(trail);
            ship_gamemode(player);
            break;
        case GAMEMODE_PLAYER_BALL: ball_gamemode(player); break;
        case GAMEMODE_BIRD:
            glitter_particles.emitterX = state.camera_x_middle;
            glitter_particles.emitterY = state.camera_y_middle;
            glitter_particles.emitting = true;
            MotionTrail_ResumeStroke(trail);
            ufo_gamemode(player);
            break;
        case GAMEMODE_DART:
            glitter_particles.emitterX = state.camera_x_middle;
            glitter_particles.emitterY = state.camera_y_middle;
            glitter_particles.emitting = true;
            MotionTrail_ResumeStroke(trail);
            wave_gamemode(player);
            break;
        case GAMEMODE_ROBOT: robot_gamemode(player); break; // Added Robot
    }
    
    player->time_since_ground += STEPS_DT;

    if (player->gamemode != GAMEMODE_DART || player->cutscene_timer > 0) {
        if (wave_trail->opacity > 0) wave_trail->opacity -= 0.02f;
        if (wave_trail->opacity <= 0) { wave_trail->opacity = 0; wave_trail->nuPoints = 0; }
    }

    if (!player->velocity_override) {
        float newVel = player->vel_y + player->gravity * STEPS_DT;
        if (!player->on_ground && state.old_player.on_ground && ((!state.input.holdJump && (state.old_input.pressedJump || state.input.pressedJump)) || player->buffering_state == BUFFER_READY) && gravBottom(&state.old_player) > gravFloor(&state.old_player) && player->mini == state.old_player.mini) {
            player->y += grav(&state.old_player, state.old_player.gravity) * STEPS_DT * STEPS_DT;
            if (player->vel_y == 0) newVel += state.old_player.gravity * STEPS_DT;
        }
        player->vel_y = newVel;
    }

    if (player->cutscene_timer > 0) return;
    player->rotation = normalize_angle(player->rotation);
    
    if (player->snap_rotation) { player->lerp_rotation = player->rotation; } 
    else {
        if (player->gamemode == GAMEMODE_BIRD) {
            if (player->slope_data.slope_id >= 0) player->lerp_rotation = iSlerp(player->lerp_rotation, player->rotation, 0.05f, STEPS_DT);
            else player->lerp_rotation = iSlerp(player->lerp_rotation, player->rotation, 0.1f, STEPS_DT);
        } else { player->lerp_rotation = iSlerp(player->lerp_rotation, player->rotation, 0.2f, STEPS_DT); }
    }

    bool slopeCheck = player->slope_data.slope_id >= 0 && (grav_slope_orient(player->slope_data.slope_id, player) == ORIENT_NORMAL_DOWN || grav_slope_orient(player->slope_data.slope_id, player) == ORIENT_UD_DOWN);

    if (getGroundBottom(player) < state.ground_y) {
        if (player->ceiling_inv_time <= 0 && player->gravObj_id < 0 && (player->gamemode == GAMEMODE_PLAYER || player->gamemode == GAMEMODE_ROBOT) && player->upside_down) state.dead = true;
        if (slopeCheck) clear_slope_data(player);
        if (player->gamemode != GAMEMODE_DART && grav(player, player->vel_y) <= 0) set_p_velocity(player, 0, player->gamemode == GAMEMODE_PLAYER_BALL);
        player->y = state.ground_y + (player->height / 2) + ((player->gamemode == GAMEMODE_DART) ? (player->mini ? 3 : 5) : 0);
    }

    if (getGroundTop(player) > state.ceiling_y) {
        if (player->ceiling_inv_time <= 0 && player->gravObj_id < 0 && (player->gamemode == GAMEMODE_PLAYER || player->gamemode == GAMEMODE_ROBOT) && !player->upside_down) state.dead = true;
        if (slopeCheck) clear_slope_data(player);
        if (player->gamemode != GAMEMODE_DART && grav(player, player->vel_y) >= 0) set_p_velocity(player, 0, player->gamemode == GAMEMODE_PLAYER_BALL);
        player->y = state.ceiling_y - (player->height / 2) - ((player->gamemode == GAMEMODE_DART) ? (player->mini ? 3 : 5) : 0);
    } 

    if (player->slope_data.slope_id >= 0) slope_calc(player->slope_data.slope_id, player);
    if (player->gamemode == GAMEMODE_SHIP) rotate_fly(player, 0.15f);
    if (player->gamemode == GAMEMODE_DART) rotate_fly(player, player->mini ? 0.4f : 0.25f);
}

// ... Keep other functions as they were ...

void draw_player(Player *player) {
    if (state.dead) return;
    float calc_x = ((player->x - state.camera_x));
    float calc_y = SCREEN_HEIGHT - ((player->y - state.camera_y));
    u32 primary_color = C2D_Color32(p1_color.r, p1_color.g, p1_color.b, 255);
    u32 secondary_color = C2D_Color32(p2_color.r, p2_color.g, p2_color.b, 255);
    if (state.current_player == 1) { u32 tmp = primary_color; primary_color = secondary_color; secondary_color = tmp; }
    float scale = (player->mini) ? 0.6f : 1.f;
    bool flip_x = (state.mirror_mult < 0);
    float p_rot = player->lerp_rotation * state.mirror_mult;
    float calc_x_mirror = get_mirror_x(calc_x, state.mirror_factor);

    switch (player->gamemode) {
        case GAMEMODE_PLAYER:
            spawn_icon_at(GAMEMODE_PLAYER, selected_cube, player_glow_enabled, calc_x_mirror, calc_y, p_rot, flip_x, false, scale, primary_color, secondary_color, C2D_Color32(glow_color.r, glow_color.g, glow_color.b, 255));
            break;
        case GAMEMODE_ROBOT:
            // Placeholder: Robot uses Cube icon until Robot Sprite Sheet is implemented
            spawn_icon_at(GAMEMODE_PLAYER, selected_cube, player_glow_enabled, calc_x_mirror, calc_y, p_rot, flip_x, false, scale, primary_color, secondary_color, C2D_Color32(glow_color.r, glow_color.g, glow_color.b, 255));
            break;
        // ... Keep other cases ...
    }
}
