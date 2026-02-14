#pragma once
#include "ui_element.h"

#define UI_MAX_ELEMENTS 64

typedef struct {
    UIElement elements[UI_MAX_ELEMENTS];
    int count;
} UIScreen;

void ui_screen_update(UIScreen* screen, touchPosition* touch);
void ui_screen_draw(UIScreen* screen);