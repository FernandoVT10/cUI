#include <ctype.h>

#include "input.h"

#define FONT_SPACING 2
#define CURSOR_BLINK_RATE 0.5 // time for the cursor to show and hide in seconds
#define CURSOR_LINE_WIDTH 2

Input *create_input(InputProps props)
{
    Input *input = malloc(sizeof(Input));
    bzero(input, sizeof(Input));

    input->pos = props.pos;
    input->size = props.size;
    input->font = props.font;
    input->font_size = props.font_size;
    input->font_color = props.font_color;
    input->placeholder = props.placeholder;
    input->padding = props.padding;
    input->border_color = props.border_color;
    input->bg_color = props.bg_color;
    input->cursor.is_collapsed = true;

    return input;
}

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
    size_t start,
    size_t end
)
{
    Vector2 size;

    if(end == 0 || end > str->count || start >= end) {
        return (Vector2) {0, 0};
    }

    if(end == str->count) {
        string_append_chr(str, '\0');
        size = MeasureTextEx(font, str->items + start, font_size, font_spacing);
        str->count--;
    } else {
        // saves the char, replaces the char with the null terminator, measures the text,
        // and then replaces back the char
        char c = str->items[end];
        str->items[end] = '\0';
        size = MeasureTextEx(font, str->items + start, font_size, font_spacing);
        str->items[end] = c;
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

static void set_cursor_selection(InputCursor *cursor, size_t start, size_t end)
{
    if(start != end) {
        cursor->is_collapsed = false;
        cursor->selection.start = start;
        cursor->selection.end = end;
    } else {
        cursor->is_collapsed = true;
        cursor->pos = start;
        cursor->blink_t = 0;
    }
}

static bool is_ctrl_down()
{
    return IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
}

// returns a vector where "x" contains the smaller value
static Vector2 get_selection_as_vec(InputSelection selection)
{
    if(selection.start > selection.end) {
        return (Vector2) {selection.end, selection.start};
    } else {
        return (Vector2) {selection.start, selection.end};
    }
}

static void remove_selected_text(Input *input)
{
    InputCursor *cursor = &input->cursor;
    if(cursor->is_collapsed) return;

    Vector2 sel_vec = get_selection_as_vec(cursor->selection);

    string_remove_slice(&input->text, sel_vec.x, sel_vec.y);
    cursor->is_collapsed = true;
    set_cursor_pos(cursor, sel_vec.x);
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
                input->cursor.is_collapsed = true;
                set_cursor_pos(&input->cursor, i);
                break;
            }

            // between characters an additional width (font spacing) is added
            if(i > 0) chr_pos += FONT_SPACING;
        }

        // if the mouse position exceeds last char position, we set the cursor to the last position
        if(mouse_pos.x - input_box.left > chr_pos) {
            input->cursor.is_collapsed = true;
            set_cursor_pos(&input->cursor, input->text.count);
        }
    }
}

static void handle_editing(Input *input)
{
    int chr;
    while((chr = GetCharPressed()) != 0) {
        if(!input->cursor.is_collapsed) {
            remove_selected_text(input);
        }
        string_insert_chr(&input->text, chr, input->cursor.pos);
        set_cursor_pos(&input->cursor, input->cursor.pos + 1);
    }

    bool is_backspace_active = IsKeyPressedRepeat(KEY_BACKSPACE) || IsKeyPressed(KEY_BACKSPACE);

    if(!input->cursor.is_collapsed && is_backspace_active) {
        remove_selected_text(input);
    } else if(is_ctrl_down() && is_backspace_active && input->cursor.pos > 0) {
        char cur_chr = input->text.items[input->cursor.pos - 1];

        if(!isalnum(cur_chr)) {
            string_remove_chr(&input->text, input->cursor.pos - 1);
            set_cursor_pos(&input->cursor, input->cursor.pos - 1);
        } else {
            size_t cur_pos = input->cursor.pos;
            while(cur_pos > 0 && isalnum(cur_chr)) {
                cur_pos--;
                if(cur_pos > 0) {
                    cur_chr = input->text.items[cur_pos - 1];
                }
            }

            string_remove_slice(&input->text, cur_pos, input->cursor.pos);
            set_cursor_pos(&input->cursor, cur_pos);
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

static void handle_arrow_keys(Input *input)
{
    bool is_right_down = IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT);
    bool is_left_down = IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT);

    InputCursor *cursor = &input->cursor;

    if(IsKeyDown(KEY_RIGHT_SHIFT) || IsKeyDown(KEY_LEFT_SHIFT)) {
        if(is_right_down) {
            if(cursor->is_collapsed && cursor->pos < input->text.count) {
                set_cursor_selection(cursor, cursor->pos, cursor->pos + 1);
            } else if(cursor->selection.end < input->text.count) {
                set_cursor_selection(
                    cursor, cursor->selection.start, cursor->selection.end + 1
                );
            }
        } else if(is_left_down) {
            if(cursor->is_collapsed && cursor->pos > 0) {
                set_cursor_selection(cursor, cursor->pos, cursor->pos - 1);
            } else if(cursor->selection.end > 0) {
                set_cursor_selection(
                    cursor, cursor->selection.start, cursor->selection.end - 1
                );
            }
        }
    } else {
        if(is_right_down) {
            if(cursor->is_collapsed && cursor->pos < input->text.count) {
                // moves cursor to the right
                set_cursor_pos(cursor, cursor->pos + 1);
            } else if(!cursor->is_collapsed) {
                // removes the selection and sets the cursor at the end of it
                InputSelection sel = cursor->selection;
                size_t pos = sel.start > sel.end ? sel.start : sel.end;
                cursor->is_collapsed = true;
                set_cursor_pos(cursor, pos);
            }
        } else if(is_left_down) {
            if(cursor->is_collapsed && cursor->pos > 0) {
                // moves the cursor to the left
                set_cursor_pos(cursor, cursor->pos - 1);
            } else if(!cursor->is_collapsed) {
                // removes the selection and sets the cursor at the start of it
                InputSelection sel = cursor->selection;
                size_t pos = sel.start > sel.end ? sel.end : sel.start;
                cursor->is_collapsed = true;
                set_cursor_pos(cursor, pos);
            }
        }
    }
}

static void update_scroll(Input *input)
{
    float text_width = measure_string_slice(
        &input->text, input->font, input->font_size, FONT_SPACING, 0, input->cursor.pos
    ).x;

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

static void draw_input_text(Input *input)
{
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

    if(input->text.count > 0) {
        draw_string(
            input->font,
            &input->text,
            text_pos,
            input->font_size,
            FONT_SPACING,
            input->font_color
        );
    } else {
        Color color = ColorAlpha(input->font_color, 0.5);
        DrawTextEx(
            input->font,
            input->placeholder,
            text_pos,
            input->font_size,
            FONT_SPACING,
            color
        );
    }
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

static void draw_selection(Input *input)
{
    InputCursor *cursor = &input->cursor;
    InputBox input_box = get_input_visible_box(input);

    Vector2 sel_vec = get_selection_as_vec(cursor->selection);

    float selection_width = measure_string_slice(
        &input->text,
        input->font,
        input->font_size,
        FONT_SPACING,
        sel_vec.x,
        sel_vec.y
    ).x;

    float start_pos = measure_string_slice(
        &input->text,
        input->font,
        input->font_size,
        FONT_SPACING,
        0,
        sel_vec.x
    ).x;

    Vector2 pos = {
        .x = input_box.left + start_pos - input->scroll,
        .y = input_box.top - 1,
    };

    // We add a little offset if the selection doesn't start from the first char
    if(sel_vec.x > 0) {
        pos.x += FONT_SPACING;
    }

    Vector2 size = {
        .x = selection_width,
        .y = input->font_size + 2,
    };
    DrawRectangleV(pos, size, ColorAlpha(input->font_color, 0.4));
}

static void draw_cursor(Input *input)
{
    InputCursor *cursor = &input->cursor;
    InputBox input_box = get_input_visible_box(input);

    cursor->blink_t += GetFrameTime();

    if(cursor->blink_t > CURSOR_BLINK_RATE * 2) {
        cursor->blink_t = 0;
    }

    if(cursor->blink_t < CURSOR_BLINK_RATE) {
        float text_width = measure_string_slice(
            &input->text,
            input->font,
            input->font_size,
            FONT_SPACING,
            0, input->cursor.pos
        ).x;

        Vector2 pos = {
            .x = input_box.left + text_width - input->scroll,
            .y = input_box.top - 1,
        };
        Vector2 size = {
            .x = CURSOR_LINE_WIDTH,
            .y = input->font_size + 2,
        };
        DrawRectangleV(pos, size, input->font_color);
    }
}

void handle_input(Input *input)
{
    handle_mouse(input);
    if(input->focused) {
        handle_editing(input);
        handle_arrow_keys(input);
        handle_clipboard(input);
        update_scroll(input);
    }

    DrawRectangleV(input->pos, input->size, input->bg_color);

    if(!input->cursor.is_collapsed && input->focused) {
        draw_selection(input);
    }

    draw_input_text(input);

    if(input->cursor.is_collapsed && input->focused) {
        draw_cursor(input);
    }
}
