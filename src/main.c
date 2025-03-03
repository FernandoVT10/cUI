#include <stdio.h>

#include "raylib.h"
#include "input.h"

#define COLOR_BG CLITERAL(Color) { 22, 20, 31, 255 }

int main(void)
{
    InitWindow(1280, 720, "cUI");
    SetTargetFPS(60);

    Vector2 input_size = { 300, 60 };
    Vector2 input_pos = {
        .x = GetScreenWidth() / 2 - input_size.x / 2,
        .y = GetScreenHeight() / 2 - input_size.y / 2,
    };

    Input input = {
        .pos = input_pos,
        .size = input_size,
        .placeholder = "This is an input",
        .font = GetFontDefault(),
        .font_size = 20,
        .font_color = WHITE,
        .padding = { 20, 20, 20, 20 },
        .border_color = RED,
        .bg_color = BLACK,
    };

    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(COLOR_BG);
        handle_input(&input);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
