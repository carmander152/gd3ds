#include "ui_screen.h"

void ui_screen_update(UIScreen* s, touchPosition* touch) {
    for (int i = 0; i < s->count; i++)
        s->elements[i].update(&s->elements[i], touch);
}

void ui_screen_draw(UIScreen* s) {
    for (int i = 0; i < s->count; i++)
        s->elements[i].draw(&s->elements[i]);
}