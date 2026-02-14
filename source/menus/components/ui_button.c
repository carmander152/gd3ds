#include "ui_element.h"
#include <citro2d.h>
#include "ui_image.h"

static void ui_button_update(UIElement* e, touchPosition* touch) {
    if (!e->enabled || !e->visible) return;

    bool pressedTouch = hidKeysDown() & KEY_TOUCH;
    bool releasedTouch = hidKeysUp() & KEY_TOUCH;

    bool inside = touch->px >= e->x - (e->w / 2) && touch->px < e->x + (e->w / 2) &&
                  touch->py >= e->y - (e->h / 2) && touch->py < e->y + (e->h / 2);

    // Check if pressed the button
    if (inside && pressedTouch) {
        e->button.hovered = true;
    }
    
    // Animation
    // TODO: replace with bounce in and bounce out
    if (e->button.hovered) {
        e->button.hoverScale += 0.05f;
        if (e->button.hoverScale > 1.1f)
            e->button.hoverScale = 1.1f;
    } else {
        e->button.hoverScale -= 0.05f;
        if (e->button.hoverScale < 1.0f)
            e->button.hoverScale = 1.0f;
    }

    // If released on button, do its action
    if (e->button.hovered && releasedTouch) {
        if (e->action)
            e->action(e->action_data);
    }
    
    // Unpress the button
    if (!inside) {
        e->button.hovered = false;
    }
}

static void ui_button_draw(UIElement* e) {
    if (!e->visible) return;

    float scale = e->button.hoverScale;

    C2D_SpriteSetCenter(&e->button.image.sprite, 0.5f, 0.5f);
    C2D_SpriteSetScale(&e->button.image.sprite, scale, scale);
    C2D_SpriteSetPos(&e->button.image.sprite, e->x, e->y);
    C2D_DrawSprite(&e->button.image.sprite);
}

UIElement ui_create_button(
    int x, int y, int sprite_index, 
    UIActionFn action,
    void* action_data
) {
    UIElement e = {
        .type = UI_BUTTON,
        .x = x, .y = y,
        .w = 0, .h = 0,
        .visible = true,
        .enabled = true,
        .action = action,
        .action_data = action_data,
        .update = ui_button_update,
        .draw = ui_button_draw
    };

    C2D_SpriteFromSheet(&e.button.image.sprite, ui_sheet, sprite_index);

    e.w = e.button.image.sprite.params.pos.w;
    e.h = e.button.image.sprite.params.pos.h;

    return e;
}