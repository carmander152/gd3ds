#include <citro2d.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#include "objects.h"
#include "objects_array.h"
#include "level_loading.h"
#include "main.h"
#include "graphics.h"
#include "state.h"
#include "player/player.h"
#include "triggers.h"

// --- Declarations for Menu & Flow ---
#include "level/main_levels.h" 
#include "menus/main_menu.h"
#include "menus/gameplay.h"
#include "menus/level_select.h"
// ------------------------------------

// --- LINKER POLYFILLS (Missing Wii Port Data) ---
bool is_citra() { return false; }
int frame_skipped = 0;
unsigned int level_frame = 0;
const float player_speeds[SPEED_COUNT] = { 251.16f, 311.58f, 387.42f, 468.0f };

// Missing Particle Arrays
ParticleSystem drag_particles[2];
ParticleSystem ship_fire_particles[2];
ParticleSystem ship_secondary_particles[2];
ParticleSystem secondary_particles[2];
ParticleSystem burst_particles[2];
ParticleSystem land_particles[2];
ParticleSystem explosion_particles[2];
ParticleSystem drag_particles_2[2];
ParticleSystem touch_drag_particles;
ParticleSystem touch_explosion_particles;
ParticleSystem coin_pickup_particles;
ParticleSystem glitter_particles;
ParticleSystem slow_speed_particles;
ParticleSystem normal_speed_particles;
ParticleSystem fast_speed_particles;
ParticleSystem faster_speed_particles;

MotionTrail trail_p1;
MotionTrail trail_p2;
MotionTrail wave_trail_p1;
MotionTrail wave_trail_p2;

// Missing Menu and Debug Functions/Variables
bool alt_title_screen = false;
bool playing_menu_loop = false;
void draw_hitbox(int obj_index) {}
void draw_player_hitbox(Player *player) {}
void draw_hitbox_trail(int param) {}
void draw_p1_trail(Player *player) {}
int output_log(const char* fmt, ...) { return 0; }

// Missing Physics Function (Crucial for the Robot's jump!)
void set_p_velocity(Player* player, float velocity, bool override) {
    player->vel_y = velocity; 
}
// ------------------------------------------------

C3D_RenderTarget* top;
C3D_RenderTarget* bot;
int game_state = STATE_MAIN_MENU;

void game_loop() {
    char *path = state.custom_level ? state.custom_level_path : main_levels[curr_level_id].gmd_path;
    int returned = load_level(path);
    if (returned) {
        game_state = (state.custom_level ? STATE_EXTERNAL_LEVELS : STATE_LEVEL_SELECT);
        return;
    }

    init_triggers(); // Start the Move Trigger system
    init_variables();

    float accumulator = 0.0f;
    u64 lastTime = svcGetSystemTick();

    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        u32 kHeld = hidKeysHeld();
        
        state.old_input = state.input;
        state.input.pressedJump = (kDown & KEY_A) || (kDown & KEY_UP);
        state.input.holdJump = (kHeld & KEY_A) || (kHeld & KEY_UP);

        u64 now = svcGetSystemTick();
        float delta = (now - lastTime) / (CPU_TICKS_PER_MSEC * 1000.0f);
        lastTime = now;
        if (delta > 0.1f) delta = STEPS_DT_UNMOD;

        if (!game_paused && !state.dead) {
            accumulator += delta;
            while (accumulator >= STEPS_DT_UNMOD) {
                state.old_player = state.player;
                handle_player(&state.player);

                // --- 2.0 TRIGGER UPDATE ---
                update_move_triggers(STEPS_DT_UNMOD);
                update_alpha_triggers(STEPS_DT_UNMOD);

                if (state.dual) {
                    state.old_player = state.player2;
                    handle_player(&state.player2);
                }
                run_camera();
                accumulator -= STEPS_DT_UNMOD;
            }
        }

        // --- RENDER ---
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_SceneBegin(top);
        C2D_TargetClear(top, C2D_Color32(0, 0, 0, 255));
        
        draw_background(state.background_x / 8, -(state.camera_y / 8) + 200);
        draw_objects();
        draw_player(&state.player);
        if (state.dual) draw_player(&state.player2);
        
        C3D_FrameEnd(0);
        if (kDown & KEY_START) break;
    }
    unload_level();
}

int main(int argc, char* argv[]) {
    romfsInit();
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(MAX_SPRITES);
    C2D_Prepare();

    top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    bot = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

    while (aptMainLoop()) {
        switch (game_state) {
            case STATE_GAME: game_loop(); break;
            case STATE_MAIN_MENU: main_menu_loop(); break;
            default: game_state = STATE_GAME; break;
        }
    }

    C2D_Fini(); C3D_Fini(); gfxExit(); romfsExit();
    return 0;
}
