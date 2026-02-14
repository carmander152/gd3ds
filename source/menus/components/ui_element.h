#pragma once
#include <stdbool.h>
#include <3ds.h>
#include <citro2d.h>

typedef enum {
    UI_BUTTON,
    UI_IMAGE
} UIElementType;

typedef struct {
    C2D_Sprite sprite;
    float scaleX;
    float scaleY;

    C2D_ImageTint tint;
    bool useTint;
} UIImageData;

typedef struct {
    UIImageData image;

    bool hovered;
    bool pressed;
    float hoverScale;
} UIButtonData;

typedef struct UIElement UIElement;

typedef void (*UIActionFn)(void* userdata);

struct UIElement {
    UIElementType type;

    int x, y;
    int w, h;

    bool visible;
    bool enabled;

    UIActionFn action;
    void* action_data;

    union {
        UIImageData image;
        UIButtonData button;
    };

    void (*update)(UIElement*, touchPosition*);
    void (*draw)(UIElement*);
};