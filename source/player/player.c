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
#include "triggers.h"

inline float gravFloor(Player *player) { return player->upside_down ? -state.ceiling_y : state.ground_y; }

void cube_gamemode(Player *player) {
    if (player->on_ground) {
        if (state.input.holdJump && player->buffering_state == BUFFER_READY) {
            set_p_velocity(player, cube_jump_heights[state.speed], false);
            player->on_ground = false;
            player->buffering_state = BUFFER_END;
        }
        player->rotation = roundf(player->rotation / 90.0f) * 90.0f;
    } else {
        player->rotation += (player->upside_down ? -380.0f : 380.0f) * STEPS_DT;
    }
}

void ship_gamemode(Player *player) {
    float ship_accel = 1200.0f;
    if (state.input.holdJump) {
        player->vel_y += grav(player, ship_accel) * STEPS_DT;
    } else {
        player->vel_y -= grav(player, ship_accel) * STEPS_DT;
    }
    player->rotation = (player->vel_y / 10.0f) * (player->upside_down ? -1 : 1);
}

void ball_gamemode(Player *player) {
    if (player->on_ground && state.input.pressedJump) {
        player->upside_down = !player->upside_down;
        player->on_ground = false;
        player->gravity = -player->gravity;
    }
    player->rotation += (player->upside_down ? -420.0f : 420.0f) * STEPS_DT;
}

void ufo_gamemode(Player *player) {
    if (state.input.pressedJump) {
        set_p_velocity(player, 550.0f, false);
    }
    player->rotation = (player->vel_y / 15.0f);
}

void wave_gamemode(Player *player) {
    float wave_speed = 600.0f;
    if (state.input.holdJump) {
        player->vel_y = grav(player, wave_speed);
    } else {
        player->vel_y = grav(player, -wave_speed);
    }
    player->rotation = (state.input.holdJump ? 45.0f : -45.0f) * (player->upside_down ? -1 : 1);
}

void robot_gamemode(Player *player) {
    if (player->vel_y < -810) player->vel_y = -810;
    
    if (player->on_ground && state.input.holdJump && player->buffering_state == BUFFER_READY) {
        set_p_velocity(player, cube_jump_heights[state.speed] * 0.6f, false);
        player->on_ground = false;
        player->buffering_state = BUFFER_END;
        player->robot_air_time = 0.f;
        player->gravity = 0; 
        player->velocity_override = true;
    }
    
    if (player->robot_air_time >= 0.35f || (!state.input.holdJump)) {   
        player->gravity = cube_accelerations[state.speed]; 
        player->velocity_override = false;
    } else if (player->buffering_state == BUFFER_END) {
        player->robot_air_time += STEPS_DT;
    }

    if (player->on_ground) {
        player->rotation = 0;
    }
}

void run_player(Player *player) {
    player->on_ground = false;
    player->on_ceiling = false;

    if (getGroundBottom(player) <= state.ground_y) {
        player->y = state.ground_y + (player->height / 2);
        player->on_ground = !player->upside_down;
        player->vel_y = 0;
    }
    if (getGroundTop(player) >= state.ceiling_y) {
        player->y = state.ceiling_y - (player->height / 2);
        player->on_ground = player->upside_down;
        player->vel_y = 0;
    }

    switch (player->gamemode) {
        case GAMEMODE_PLAYER:      cube_gamemode(player); break;
        case GAMEMODE_SHIP:        ship_gamemode(player); break;
        case GAMEMODE_PLAYER_BALL: ball_gamemode(player); break;
        case GAMEMODE_BIRD:        ufo_gamemode(player); break;
        case GAMEMODE_DART:        wave_gamemode(player); break;
        case GAMEMODE_ROBOT:       robot_gamemode(player); break;
    }

    if (!player->velocity_override) {
        player->vel_y += player->gravity * STEPS_DT;
    }
    player->y += player->vel_y * STEPS_DT;
    player->x += player->vel_x * STEPS_DT;

    player->lerp_rotation = iSlerp(player->lerp_rotation, player->rotation, 0.2f, STEPS_DT);
}

void draw_robot(Player *player, float x, float y, float scale, u32 col1, u32 col2) {
    bool flip = (state.mirror_mult < 0);
    float p_rot = player->lerp_rotation;
    float leg_rot = 0;
    if (player->on_ground) {
        leg_rot = sinf(player->x * 0.15f) * 40.0f;
    }

    // Replace indices with your actual generated icons.h constants!
    /*
    spawn_icon_part(robot_01_03_idx, x, y, p_rot + leg_rot, scale, col1, col2, flip);
    spawn_icon_part(robot_01_02_idx, x, y, p_rot, scale, col1, col2, flip);
    spawn_icon_part(robot_01_01_idx, x, y, p_rot, scale, col1, col2, flip);
    spawn_icon_part(robot_01_04_idx, x, y, p_rot - leg_rot, scale, col1, col2, flip);
    */
    C2D_DrawRectangle(x - 15, y - 15, 0, 30, 30, col1, col1, col1, col1);
}

void draw_player(Player *player) {
    if (state.dead) return;
    
    float x = player->x - state.camera_x;
    float y = SCREEN_HEIGHT - (player->y - state.camera_y);
    
    u32 c1 = C2D_Color32(p1_color.r, p1_color.g, p1_color.b, 255);
    u32 c2 = C2D_Color32(p2_color.r, p2_color.g, p2_color.b, 255);
    
    float scale = (player->mini) ? 0.6f : 1.0f;

    if (player->gamemode == GAMEMODE_ROBOT) {
        draw_robot(player, x, y, scale, c1, c2);
    } else {
        spawn_icon_at(player->gamemode, selected_cube, true, x, y, player->lerp_rotation, (state.mirror_mult < 0), player->upside_down, scale, c1, c2, 0xFFFFFFFF);
    }
}

void handle_player(Player *player) {
    if (state.dead) return;
    if (state.input.pressedJump) player->buffering_state = BUFFER_READY;
    run_player(player);
    if (!state.input.holdJump) player->buffering_state = BUFFER_NONE;
}
