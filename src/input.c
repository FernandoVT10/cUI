#include <ctype.h>

#include "input.h"

#define FONT_SPACING 2
#define CURSOR_BLINK_RATE 0.5 // time for the cursor to show and hide in seconds

// similar to DrawTextEx from raylib
static void draw_string(
    Font font,
    String *str,
    Vector2 pos,
    float font_size,
    float spacing,
    Color color
)
{
    string_append_chr(str, '\0');
    DrawTextEx(font, str->items, pos, font_size, spacing, color);
    str->count--; // remove appended char
}

// measures a String from the beginning until "pos"
static Vector2 measure_string_slice(
    String *str,
    Font font,
    int font_size,
    int font_spacing,
    size_t pos
)
{
    Vector2 size;

    if(pos == 0 || pos > str->count) {
        return (Vector2) {0, 0};
    }

    if(pos == str->count) {
        string_append_chr(str, '\0');
        size = MeasureTextEx(font, str->items, font_size, font_spacing);
        str->count--;
    } else {
        // saves the char, replaces the char with the null terminator, measures the text,
        // and then replaces back the char
        char c = str->items[pos];
        str->items[pos] = '\0';
        size = MeasureTextEx(font, str->items, font_size, font_spacing);
        str->items[pos] = c;
    }

    return size;
}

typedef struct {
    float left;
    float right;
    float top;
    float bottom;
} InputBox;

// creates a bounding box of the insides of the input
static InputBox get_input_visible_box(Input *input)
{
    return (InputBox) {
        .left = input->pos.x + input->padding.left,
        .right = input->pos.x + input->size.x - input->padding.right,
        .top = input->pos.y + input->padding.top,
        .bottom = input->pos.y + input->size.y - input->padding.bottom,
    };
}

static void set_cursor_pos(InputCursor *cursor, size_t pos)
{
    cursor->pos = pos;
    cursor->blink_t = 0;
}

static void handle_mouse(Input *input)
{
    Vector2 mouse_pos = GetMousePosition();
    Rectangle input_rect =  {
        input->pos.x, input->pos.y,
        input->size.x, input->size.y,
    };
    input->hovered = CheckCollisionPointRec(mouse_pos, input_rect);

    if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && input->hovered) {
        input->focused = true;
        // reset cursor's blinking
        input->cursor.blink_t = 0;
    } else if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && !input->hovered) {
        input->focused = false;
    }

    InputBox input_box = get_input_visible_box(input);

    if(input->hovered
        && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)
        && (mouse_pos.y > input_box.top && mouse_pos.y < input_box.top + input->font_size)) {

        // helper string for measuring the letters with raylib
        char s[2];
        s[1] = '\0';
        float chr_pos = 0;

        for(size_t i = 0; i < input->text.count; i++) {
            s[0] = input->text.items[i];

            chr_pos += MeasureTextEx(input->font, s, input->font_size, FONT_SPACING).x;

            if(chr_pos > mouse_pos.x - input_box.left) {
                set_cursor_pos(&input->cursor, i);
                break;
            }

            // between characters an additional width (font spacing) is added
            if(i > 0) chr_pos += FONT_SPACING;
        }

        // if the mouse position exceeds last char position, we set the cursor to the last position
        if(mouse_pos.x - input_box.left > chr_pos) {
            set_cursor_pos(&input->cursor, input->text.count);
        }
    }
}

static bool is_ctrl_down()
{
    return IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
}

static void handle_editing(Input *input)
{
    if(!input->focused) return;

    int chr;
    while((chr = GetCharPressed()) != 0) {
        string_insert_chr(&input->text, chr, input->cursor.pos);
        set_cursor_pos(&input->cursor, input->cursor.pos + 1);
    }

    bool is_backspace_active = IsKeyPressedRepeat(KEY_BACKSPACE) || IsKeyPressed(KEY_BACKSPACE);

    if(is_ctrl_down() && is_backspace_active && input->cursor.pos > 0) {
        char cur_chr = input->text.items[input->cursor.pos - 1];

        if(!isalnum(cur_chr)) {
            string_remove_chr(&input->text, input->cursor.pos - 1);
            set_cursor_pos(&input->cursor, input->cursor.pos - 1);
        } else {
            while(input->cursor.pos > 0 && isalnum(cur_chr)) {
                // TODO: optimize this code to remove all the necessary characters at once
                string_remove_chr(&input->text, input->cursor.pos - 1);
                set_cursor_pos(&input->cursor, input->cursor.pos - 1);

                if(input->cursor.pos > 0) {
                    cur_chr = input->text.items[input->cursor.pos - 1];
                }
            }
        }
    } else if(input->cursor.pos > 0 && is_backspace_active) {
        string_remove_chr(&input->text, input->cursor.pos - 1);
        set_cursor_pos(&input->cursor, input->cursor.pos - 1);
    }
}

static void handle_clipboard(Input *input)
{
    if(is_ctrl_down() && IsKeyPressed(KEY_V)) {
        const char *raw = GetClipboardText();

        if(strlen(raw) > 0) {
            char *formatted_text = malloc(strlen(raw) + 1);
            size_t i = 0;

            // removes new lines from pasted text
            for(size_t j = 0; j <= strlen(raw); j++) {
                if(raw[j] != '\n') {
                    formatted_text[i++] = raw[j];
                }
            }

            string_insert_text(&input->text, formatted_text, input->cursor.pos);
            set_cursor_pos(&input->cursor, input->cursor.pos + strlen(formatted_text));

            free(formatted_text);
        }
    }
}

static void update_scroll(Input *input, float text_width)
{
    float pos_x = text_width - input->scroll;
    InputBox input_box = get_input_visible_box(input);
    float input_size = input_box.right - input_box.left;

    if(pos_x > input_size) {
        input->scroll += pos_x - input_size;
    } else if(pos_x < 0) {
        // NOTE: here the scroll variable is being substracted
        input->scroll += pos_x;
    }
}

static void update_cursor(Input *input)
{
    if(!input->focused) return;

    if((IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT)) && input->cursor.pos < input->text.count) {
        set_cursor_pos(&input->cursor, input->cursor.pos + 1);
    } else if((IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT)) && input->cursor.pos > 0) {
        set_cursor_pos(&input->cursor, input->cursor.pos - 1);
    }

    Vector2 text_size = measure_string_slice(
        &input->text, input->font, input->font_size, FONT_SPACING, input->cursor.pos
    );

    update_scroll(input, text_size.x);

    InputBox input_box = get_input_visible_box(input);
    input->cursor.draw_pos = (Vector2) {
        .x = input_box.left + text_size.x - input->scroll,
        .y = input_box.top - 1,
    };
}

static Vector2 get_cursor_size(Input *input)
{
    return (Vector2) {
        .x = 2,
        .y = input->font_size + 2,
    };
}

static void draw_input(Input *input)
{
    DrawRectangleV(input->pos, input->size, input->bg_color);

    InputBox input_box = get_input_visible_box(input);
    BeginScissorMode(
        input_box.left,
        input_box.top,
        input_box.right - input_box.left,
        input_box.bottom - input_box.top
    );

    Vector2 text_pos = {
        .x = input_box.left - input->scroll,
        .y = input_box.top,
    };

    draw_string(
        input->font,
        &input->text,
        text_pos,
        input->font_size,
        FONT_SPACING,
        input->font_color
    );
    EndScissorMode();

    if(input->focused) {
        int border_size = 2;
        Rectangle input_rect =  {
            input->pos.x, input->pos.y,
            input->size.x, input->size.y,
        };
        DrawRectangleLinesEx(input_rect, border_size, input->border_color);
    }
}

static void draw_cursor(Input *input)
{
    if(!input->focused) return;

    InputCursor *cursor = &input->cursor;
    cursor->blink_t += GetFrameTime();

    if(cursor->blink_t > CURSOR_BLINK_RATE * 2) {
        cursor->blink_t = 0;
    }

    if(cursor->blink_t < CURSOR_BLINK_RATE) {
        DrawRectangleV(cursor->draw_pos, get_cursor_size(input), input->font_color);
    }
}

void handle_input(Input *input)
{
    handle_mouse(input);
    handle_editing(input);
    handle_clipboard(input);
    update_cursor(input);
    draw_input(input);
    draw_cursor(input);
}
