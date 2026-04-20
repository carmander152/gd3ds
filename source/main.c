#include <citro2d.h>
#include "state.h"
#include "triggers.h"

// ... existing setup ...

void game_loop() {
    // ... loading level ...
    init_triggers();
    init_variables();

    u64 lastTime = svcGetSystemTick();
    float accumulator = 0.0f;

    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        u32 kHeld = hidKeysHeld();
        
        state.old_input = state.input;
        state.input.pressedJump = (kDown & KEY_A) || (kDown & KEY_UP);
        state.input.holdJump = (kHeld & KEY_A) || (kHeld & KEY_UP);

        u64 now = svcGetSystemTick();
        delta = (now - lastTime) / (CPU_TICKS_PER_MSEC * 1000.0f);
        lastTime = now;

        if (!game_paused && !state.dead) {
            accumulator += delta;
            while (accumulator >= STEPS_DT_UNMOD) {
                handle_player(&state.player);
                update_move_triggers(STEPS_DT_UNMOD); // UPDATED
                
                if (state.dual) handle_player(&state.player2);
                run_camera();
                accumulator -= STEPS_DT_UNMOD;
            }
        }

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_SceneBegin(top);
        C2D_TargetClear(top, C2D_Color32(0, 0, 0, 255));
        
        draw_objects();
        draw_player(&state.player);
        
        C3D_FrameEnd(0);
        if (kDown & KEY_START) break;
    }
    unload_level();
}
