#include "config.h"

#include <3ds.h>
#include "main.h"
#include "graphics.h"

#include <sys/stat.h>
#include <sys/types.h>

#include "utils/gfx.h"
#include "menus/icon_kit.h"

Config cfg;

void cfg_init() {
    // Make the directories
    mkdir(CONFIG_PARENT, 0777);
    mkdir(CONFIG_ROOT, 0777);

    config_load(&cfg, CONFIG_FILE);

    config_set_bool(&cfg, CONFIG_GRAPHICS_PATH "aaEnabled", config_get_bool(&cfg, CONFIG_GRAPHICS_PATH "aaEnabled", false));
    config_set_bool(&cfg, CONFIG_GRAPHICS_PATH "wideEnabled", config_get_bool(&cfg, CONFIG_GRAPHICS_PATH "wideEnabled", false));
    config_set_bool(&cfg, CONFIG_GRAPHICS_PATH "glowEnabled", config_get_bool(&cfg, CONFIG_GRAPHICS_PATH "glowEnabled", true));

    config_set_int(&cfg, CONFIG_CUSTOMIZATION_PATH "cube", config_get_int(&cfg, CONFIG_CUSTOMIZATION_PATH "cube", 1));
    config_set_int(&cfg, CONFIG_CUSTOMIZATION_PATH "ship", config_get_int(&cfg, CONFIG_CUSTOMIZATION_PATH "ship", 1));
    config_set_int(&cfg, CONFIG_CUSTOMIZATION_PATH "ball", config_get_int(&cfg, CONFIG_CUSTOMIZATION_PATH "ball", 1));
    config_set_int(&cfg, CONFIG_CUSTOMIZATION_PATH "ufo",  config_get_int(&cfg, CONFIG_CUSTOMIZATION_PATH "ufo",  1));
    config_set_int(&cfg, CONFIG_CUSTOMIZATION_PATH "wave", config_get_int(&cfg, CONFIG_CUSTOMIZATION_PATH "wave", 1));
    config_set_int(&cfg, CONFIG_CUSTOMIZATION_PATH "p1",   config_get_int(&cfg, CONFIG_CUSTOMIZATION_PATH "p1",   DEFAULT_P1));
    config_set_int(&cfg, CONFIG_CUSTOMIZATION_PATH "p2",   config_get_int(&cfg, CONFIG_CUSTOMIZATION_PATH "p2",   DEFAULT_P2));
    config_set_int(&cfg, CONFIG_CUSTOMIZATION_PATH "glow", config_get_int(&cfg, CONFIG_CUSTOMIZATION_PATH "glow", DEFAULT_GLOW));
    config_set_bool(&cfg, CONFIG_CUSTOMIZATION_PATH "playerGlowEnabled", config_get_bool(&cfg, CONFIG_CUSTOMIZATION_PATH "playerGlowEnabled", false));

    set_aa(config_get_bool(&cfg, CONFIG_GRAPHICS_PATH "aaEnabled", false));
    set_wide(config_get_bool(&cfg, CONFIG_GRAPHICS_PATH "wideEnabled", false));
    glowEnabled = config_get_bool(&cfg, CONFIG_GRAPHICS_PATH "glowEnabled", true);

    selected_cube = config_get_int(&cfg, CONFIG_CUSTOMIZATION_PATH "cube", 1);
    selected_ship = config_get_int(&cfg, CONFIG_CUSTOMIZATION_PATH "ship", 1);
    selected_ball = config_get_int(&cfg, CONFIG_CUSTOMIZATION_PATH "ball", 1);
    selected_ufo  = config_get_int(&cfg, CONFIG_CUSTOMIZATION_PATH "ufo",  1);
    selected_wave = config_get_int(&cfg, CONFIG_CUSTOMIZATION_PATH "wave", 1);
    selected_p1   = config_get_int(&cfg, CONFIG_CUSTOMIZATION_PATH "p1",   DEFAULT_P1);
    selected_p2   = config_get_int(&cfg, CONFIG_CUSTOMIZATION_PATH "p2",   DEFAULT_P2);
    selected_glow = config_get_int(&cfg, CONFIG_CUSTOMIZATION_PATH "glow", DEFAULT_GLOW);
    player_glow_enabled = config_get_bool(&cfg, CONFIG_CUSTOMIZATION_PATH "playerGlowEnabled", false);

    config_save(&cfg);
}

void cfg_save() {
    config_set_bool(&cfg, CONFIG_GRAPHICS_PATH "aaEnabled", aaEnabled);
    config_set_bool(&cfg, CONFIG_GRAPHICS_PATH "wideEnabled", wideEnabled);
    config_set_bool(&cfg, CONFIG_GRAPHICS_PATH "glowEnabled", glowEnabled);

    config_set_int(&cfg, CONFIG_CUSTOMIZATION_PATH "cube", selected_cube);
    config_set_int(&cfg, CONFIG_CUSTOMIZATION_PATH "ship", selected_ship);
    config_set_int(&cfg, CONFIG_CUSTOMIZATION_PATH "ball", selected_ball);
    config_set_int(&cfg, CONFIG_CUSTOMIZATION_PATH "ufo",  selected_ufo );
    config_set_int(&cfg, CONFIG_CUSTOMIZATION_PATH "wave", selected_wave);
    config_set_int(&cfg, CONFIG_CUSTOMIZATION_PATH "p1",   selected_p1  );
    config_set_int(&cfg, CONFIG_CUSTOMIZATION_PATH "p2",   selected_p2  );
    config_set_int(&cfg, CONFIG_CUSTOMIZATION_PATH "glow", selected_glow);
    config_set_bool(&cfg, CONFIG_CUSTOMIZATION_PATH "playerGlowEnabled", player_glow_enabled);

    config_save(&cfg);
}

void cfg_fini() {
    config_free(&cfg);
}