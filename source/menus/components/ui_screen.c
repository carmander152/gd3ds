#include "ui_screen.h"
#include "ui_element.h"
#include "ui_button.h"
#include "ui_image.h"
#include "ui_label.h"
#include "ui_screen.h"
#include "ui_checkbox.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <3ds.h>
#include <citro2d.h>

void ui_screen_update(UIScreen* s, touchPosition* touch) {
    for (int i = 0; i < s->count; i++)
        s->elements[i].update(&s->elements[i], touch);
}

void ui_screen_draw(UIScreen* s) {
    for (int i = 0; i < s->count; i++)
        s->elements[i].draw(&s->elements[i]);
}

UIActionFn ui_find_action(const UIAction* actions, size_t count, const char* name) {
    for (size_t i = 0; i < count; i++)
        if (strcmp(actions[i].name, name) == 0)
            return actions[i].fn;
    return NULL;
}

static void trim_newline(char* s) {
    size_t len = strlen(s);
    if (len > 0 && s[len - 1] == '\n')
        s[len - 1] = '\0';
}


static void strip_quotes(char* s) {
    size_t len = strlen(s);
    if (len >= 2 && s[0] == '"' && s[len - 1] == '"') {
        memmove(s, s + 1, len - 1);
        s[len - 2] = '\0';
    }
}

static char* next_token(char** cursor)
{
    if (!*cursor) return NULL;

    char* s = *cursor;

    // Skip leading spaces
    while (*s == ' ') s++;

    if (*s == '\0') {
        *cursor = NULL;
        return NULL;
    }

    char* start = s;
    int inQuotes = 0;

    while (*s) {
        if (*s == '"') {
            inQuotes = !inQuotes;
        }
        else if ((*s == ' ' || *s == '\n' || *s == '\r') && !inQuotes) {
            break;
        }
        s++;
    }

    if (*s) {
        *s = '\0';
        *cursor = s + 1;
    } else {
        *cursor = NULL;
    }

    return start;
}

static bool get_bool(char *value) {
    return *value == 't' || *value == 'T';
}

UIElement *get_element_by_tag(UIScreen *screen, const char *tag) {
    for (int i = 0; i < screen->count; i++) {
        if (strcmp(screen->elements[i].tag, tag) == 0) {
            return &screen->elements[i];
        }
    }
    return NULL;
}

void ui_load_screen(UIScreen* screen,
                    const UIAction* actions,
                    size_t actionCount,
                    const char* path)
{
    FILE* f = fopen(path, "r");
    if (!f) return;

    screen->count = 0;

    char line[512];

    while (fgets(line, sizeof(line), f)) {

        trim_newline(line);

        if (line[0] == '#' || line[0] == '\0')
            continue;
        
        char* cursor = line;
        char* token = next_token(&cursor);
        if (!token) continue;

        char type[16];
        strncpy(type, token, 15);

        // Defaults (optional values)
        int x = 0, y = 0, id = 0;
        float sx = 1.0f, sy = 1.0f, scale = 1.0f;
        char actionName[64] = {0};
        float align = 0.f;
        bool checked = false;

        char text[256] = {0};
        char tag[16] = {0};

        // Parse key=value pairs
        while ((token = next_token(&cursor)) != NULL) {
            char* equal = strchr(token, '=');
            if (!equal) continue;

            *equal = '\0';

            char* key = token;
            char* value = equal + 1;

            // Parameters
            if (strcmp(key, "x") == 0)
                x = atoi(value);
            else if (strcmp(key, "y") == 0)
                y = atoi(value);
            else if (strcmp(key, "id") == 0)
                id = atoi(value);
            else if (strcmp(key, "sx") == 0)
                sx = atof(value);
            else if (strcmp(key, "sy") == 0)
                sy = atof(value);
            else if (strcmp(key, "scale") == 0)
                scale = atof(value);
            else if (strcmp(key, "action") == 0) {
                strip_quotes(value);
                strncpy(actionName, value, 63);
            } else if (strcmp(key, "text") == 0) {
                strip_quotes(value);
                strncpy(text, value, 255);
            } else if (strcmp(key, "align") == 0) {
                if (strcmp(value, "LEFT") == 0) {
                    align = 0.f;
                } else if (strcmp(value, "CENTER") == 0) {
                    align = 0.5f;
                } else if (strcmp(value, "RIGHT") == 0) {
                    align = 1.0f;
                } else {
                    align = 0.f;
                } 
            } else if (strcmp(key, "tag") == 0) {
                strip_quotes(value);
                strncpy(tag, value, 15);
            } else if (strcmp(key, "checked") == 0) {
                checked = get_bool(value);
            }
        }

        if (screen->count >= UI_MAX_ELEMENTS)
            break;

        if (strcmp(type, "button") == 0) {
            screen->elements[screen->count++] =
                ui_create_button(
                    x, y, id,
                    ui_find_action(actions, actionCount, actionName),
                    NULL,
                    text,
                    tag
                );
        } else if (strcmp(type, "image") == 0) {
            screen->elements[screen->count++] =
                ui_create_image(x, y, id, sx, sy, tag);
        } else if (strcmp(type, "label") == 0) {
            screen->elements[screen->count++] =
                ui_create_label(x, y, scale, text, align, tag);
        } else if (strcmp(type, "checkbox") == 0) {
            screen->elements[screen->count++] =
                ui_create_checkbox(
                    x, y, checked,
                    ui_find_action(actions, actionCount, actionName),
                    NULL,
                    tag
                );
        }
    }

    fclose(f);
}
