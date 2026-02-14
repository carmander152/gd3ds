#pragma once
#include <3ds.h>
#include "ui_element.h"
#include "ui_screen.h"

typedef struct {
    const char* name;
    UIActionFn fn;
} UIAction;

void ui_load_screen(UIScreen* screen, const UIAction* actions, size_t count, const char* path);