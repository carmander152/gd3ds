#include "object_particles.h"
#include "state.h"
#include "main.h"

ObjectParticles object_particle[MAX_OBJECT_PS];

void init_op_system() {
    for (size_t i = 0; i < MAX_OBJECT_PS; i++) {
        object_particle[i].id = -1;
        object_particle[i].occupied = false;
    }
}

bool is_ps_already_loaded(int id) {
    for (size_t i = 0; i < MAX_OBJECT_PS; i++) {
        if (object_particle[i].occupied && object_particle[i].id == id) {
            return true;
        }
    }
    return false;
}

int load_object_particles(int id, const ParticleDefinition *cfg, bool stationary) {
    for (size_t i = 0; i < MAX_OBJECT_PS; i++) {
        if (!object_particle[i].occupied) {
            object_particle[i].occupied = true;
            object_particle[i].id = id;
            object_particle[i].isStationary = stationary;
            initParticleSystem(&object_particle[i].ps, cfg);
            return i;
        }
    }
    return -1;
}

static void remove_offscreen_object_particles() {
    for (size_t i = 0; i < MAX_OBJECT_PS; i++) {
        if (object_particle[i].occupied) {
            float x = object_particle[i].ps.emitterX;
            float y = object_particle[i].ps.emitterY;

            if (!object_particle[i].isStationary) {
                x = ((x - state.camera_x));
                y = SCREEN_HEIGHT - ((y - state.camera_y));  
            }

            if (x < -60 || x >= (SCREEN_WIDTH / SCALE) + 60 || y < -60 || y >= (SCREEN_HEIGHT / SCALE) + 60) {
                object_particle[i].occupied = false;
                freeParticleData(&object_particle[i].ps.data);
            }
        }
    }
}

void draw_object_particles() {
    remove_offscreen_object_particles();
    for (size_t i = 0; i < MAX_OBJECT_PS; i++) {
        if (object_particle[i].occupied) {
            updateParticleSystem(&object_particle[i].ps, DT);

            int obj = object_particle[i].id;
            int fade_x = 0;
            int fade_y = 0;

            float calc_x = ((objects.x[obj] - state.camera_x));

            float fade_scale = 1.f;
            get_fade_vars(obj, calc_x, &fade_x, &fade_y, &fade_scale);

            float opacity = obj_edge_fade(calc_x, SCREEN_WIDTH / SCALE) / 255.f;

            drawParticleSystem(&object_particle[i].ps, object_particle[i].isStationary, fade_x, fade_y, opacity);
        }
    }
}