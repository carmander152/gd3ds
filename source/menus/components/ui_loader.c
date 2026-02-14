#include "ui_loader.h"
#include "ui_screen.h"
#include "ui_button.h"
#include "ui_image.h"
#include <string.h>
#include <stdio.h>
#include <3ds.h>
#include <citro2d.h>

UIActionFn ui_find_action(const UIAction* actions, size_t count, const char* name) {
    for (size_t i = 0; i < count; i++)
        if (strcmp(actions[i].name, name) == 0)
            return actions[i].fn;
    return NULL;
}

void ui_load_screen(UIScreen* screen, const UIAction* actions, size_t count, const char* path) {
    FILE* f = fopen(path, "r");

    screen->count = 0;

    char line[256];

    while (fgets(line, sizeof(line), f)) {

        char type[16];

        // Read only the type first
        if (sscanf(line, "%15s", type) != 1)
            continue;

        if (strcmp(type, "button") == 0) {

            int x, y, id;
            char action[32];

            if (sscanf(line, "%*s %d %d %d %31s",
                    &x, &y, &id, action) == 4) {

                screen->elements[screen->count++] =
                    ui_create_button(
                        x, y, id,
                        ui_find_action(actions, count, action),
                        NULL
                    );
            }
        }

        else if (strcmp(type, "image") == 0) {

            int x, y, id;
            float sx, sy;

            if (sscanf(line, "%*s %d %d %d %f %f",
                    &x, &y, &id, &sx, &sy) == 5) {

                screen->elements[screen->count++] =
                    ui_create_image(x, y, id, sx, sy);
            }
        }
    }
    fclose(f);
}