#include "player.h"
#include "state.h"
#include "icons.h"
#include "graphics.h"
#include "math_helpers.h"
#include "triggers.h"

inline float gravFloor(Player *player) { return player->upside_down ? -state.ceiling_y : state.ground_y; }

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
}

// Helper to draw icons using your IconPart structure
void draw_icon(int mode, int id, float x, float y, float rot, float scale, u32 c1, u32 c2) {
    const Icon* icon = &icons[mode][id];
    for (int i = 0; i < icon->part_count; i++) {
        const IconPart* part = &icon->parts[i];
        u32 color = (part->color_type == 0) ? c1 : c2;
        // Use C2D_DrawSprite with part data here
        // This is a simplified call; adjust based on your actual sprite renderer
        draw_part_to_screen(part->texture, x + part->x, y + part->y, rot + part->rot, scale * part->scale_x, color);
    }
}

void draw_player(Player *player) {
    if (state.dead) return;
    float x = player->x - state.camera_x;
    float y = SCREEN_HEIGHT - (player->y - state.camera_y);
    u32 c1 = C2D_Color32(p1_color.r, p1_color.g, p1_color.b, 255);
    u32 c2 = C2D_Color32(p2_color.r, p2_color.g, p2_color.b, 255);
    float s = player->mini ? 0.6f : 1.0f;

    int selected_id = 0; // Logic for selected_cube, selected_robot, etc.
    draw_icon(player->gamemode, selected_id, x, y, player->lerp_rotation, s, c1, c2);
}

void run_player(Player *player) {
    player->on_ground = (getGroundBottom(player) <= state.ground_y);
    if (player->on_ground) {
        player->y = state.ground_y + (player->height / 2);
        player->vel_y = 0;
    }

    switch (player->gamemode) {
        case GAMEMODE_PLAYER: /* cube logic */ break;
        case GAMEMODE_ROBOT:  robot_gamemode(player); break;
        // Other modes...
    }

    if (!player->velocity_override) player->vel_y += player->gravity * STEPS_DT;
    player->y += player->vel_y * STEPS_DT;
    player->x += player->vel_x * STEPS_DT;
    player->lerp_rotation = iSlerp(player->lerp_rotation, player->rotation, 0.2f, STEPS_DT);
}

void handle_player(Player *player) {
    if (state.input.pressedJump) player->buffering_state = BUFFER_READY;
    run_player(player);
    if (!state.input.holdJump) player->buffering_state = BUFFER_NONE;
}
