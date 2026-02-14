#pragma once
#include "ui_element.h"

UIElement ui_create_button(
    int x, int y, int sprite_index, 
    UIActionFn action,
    void* action_data
);