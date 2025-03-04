#ifndef INPUT_H
#define INPUT_H

#include "cTooling.h"
#include "raylib.h"

typedef struct {
    int left;
    int right;
    int top;
    int bottom;
} Padding;

typedef struct {
    size_t pos;
    Vector2 draw_pos;
    float blink_t;
} InputCursor;

typedef struct {
    Vector2 pos;
    Vector2 size;
    String text;
    Font font;
    int font_size;
    Color font_color;
    const char *placeholder;
    Padding padding;
    bool focused;
    bool hovered;
    InputCursor cursor;
    int scroll;
    Color border_color;
    Color bg_color;
} Input;

typedef struct {
    Vector2 pos;
    Vector2 size;
    Font font;
    int font_size;
    Color font_color;
    const char *placeholder;
    Padding padding;
    Color border_color;
    Color bg_color;
} InputProps;

Input *create_input(InputProps props);
void handle_input(Input *input);

#endif // INPUT_H
