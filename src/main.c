#include <stdio.h>

#include "raylib.h"
#include "input.h"

#define COLOR_BG CLITERAL(Color) { 22, 20, 31, 255 }
#define COLOR_INPUT_FONT CLITERAL(Color) { 224, 222, 244, 255 }
#define COLOR_INPUT_BORDER CLITERAL(Color) { 157, 207, 216, 255 }
#define COLOR_INPUT_BG CLITERAL(Color) { 33, 32, 46, 255 }

int main(void)
{
    InitWindow(1280, 720, "cUI");
    SetTargetFPS(60);

    Vector2 input_size = { 600, 60 };
    Vector2 input_pos = {
        .x = GetScreenWidth() / 2 - input_size.x / 2,
        .y = GetScreenHeight() / 2 - input_size.y / 2,
    };

    Input *input = create_input((InputProps) {
        .pos = input_pos,
        .size = input_size,
        .placeholder = "This is an input",
        .font = GetFontDefault(),
        .font_size = 20,
        .font_color = COLOR_INPUT_FONT,
        .padding = { 20, 20, 20, 20 },
        .border_color = COLOR_INPUT_BORDER,
        .bg_color = COLOR_INPUT_BG,
    });

    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(COLOR_BG);
        handle_input(input);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
