#include "player.h"
#include "state.h"
#include "graphics.h"
#include "triggers.h"
#include "math_helpers.h"

void robot_gamemode(Player *player) {
    if (player->vel_y < -810) player->vel_y = -810;
    
    // Jump Initiation
    if (player->on_ground && state.input.holdJump && player->buffering_state == BUFFER_READY) {
        set_p_velocity(player, cube_jump_heights[state.speed] * 0.6f, false);
        player->on_ground = false;
        player->buffering_state = BUFFER_END;
        player->robot_air_time = 0.f;
        player->gravity = 0; 
        player->velocity_override = true;
    }
    
    // Thrust Logic (Holding the button)
    if (player->robot_air_time >= 0.35f || (!state.input.holdJump)) {   
        player->gravity = cube_accelerations[state.speed]; 
        player->velocity_override = false;
    } else if (player->buffering_state == BUFFER_END) {
        player->robot_air_time += STEPS_DT;
    }
}

// Modular Drawing logic for the Robot
void draw_robot(Player *player, float x, float y, float scale, u32 col1, u32 col2) {
    bool flip = (state.mirror_mult < 0);
    float p_rot = player->lerp_rotation;
    
    // Simple walk cycle math
    float leg_rot = 0;
    if (player->on_ground) {
        leg_rot = sinf(player->x * 0.15f) * 40.0f;
    }

    // You will need to map these to the actual C2D_Sprite IDs generated in icons.h
    // Example order: Back Leg -> Body -> Head -> Front Leg
    // Note: I'm assuming you have a spawn_part helper, if not, call C2D_DrawSprite directly
    
    // spawn_icon_part(robot_01_03_001, x, y, p_rot + leg_rot, scale, col1, col2, flip); // Back Leg
    // spawn_icon_part(robot_01_02_001, x, y, p_rot, scale, col1, col2, flip);           // Body
    // spawn_icon_part(robot_01_01_001, x, y, p_rot, scale, col1, col2, flip);           // Head
    // spawn_icon_part(robot_01_04_001, x, y, p_rot - leg_rot, scale, col1, col2, flip); // Front Leg
}

void draw_player(Player *player) {
    if (state.dead) return;
    float x = player->x - state.camera_x;
    float y = SCREEN_HEIGHT - (player->y - state.camera_y);
    u32 c1 = C2D_Color32(p1_color.r, p1_color.g, p1_color.b, 255);
    u32 c2 = C2D_Color32(p2_color.r, p2_color.g, p2_color.b, 255);
    
    if (player->gamemode == GAMEMODE_ROBOT) {
        draw_robot(player, x, y, 1.0f, c1, c2);
    } else {
        spawn_icon_at(player->gamemode, selected_cube, true, x, y, player->lerp_rotation, (state.mirror_mult < 0), player->upside_down, 1.0f, c1, c2, 0xFFFFFFFF);
    }
}
