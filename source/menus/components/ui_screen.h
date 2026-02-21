#pragma once
#include "ui_element.h"

#define UI_MAX_ELEMENTS 64

typedef struct {
    UIElement elements[UI_MAX_ELEMENTS];
    int count;
} UIScreen;

typedef struct {
    const char* name;
    UIActionFn fn;
} UIAction;

void ui_load_screen(UIScreen* screen, const UIAction* actions, size_t count, const char* path);

void ui_screen_update(UIScreen* screen, touchPosition* touch);
void ui_screen_draw(UIScreen* screen);
UIElement *get_element_by_tag(UIScreen *screen, const char *tag);