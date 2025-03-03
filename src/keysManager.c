#include <stdbool.h>

#include "raylib.h"
#include "keysManager.h"

#define REPEAT_KEY_INITIAL_DELAY 0.350
#define REPEAT_KEY_TICK_RATE 0.05

typedef struct {
    bool val;
    float time;
    int key;
} KeyManager;

static struct {
    KeyManager backspace;
    KeyManager right_arrow;
    KeyManager left_arrow;
} keys_manager = {
    .backspace = { .key = KEY_BACKSPACE },
    .right_arrow = { .key = KEY_RIGHT },
    .left_arrow = { .key = KEY_LEFT },
};

static void update_key(KeyManager *key_manager)
{
    if(IsKeyPressed(key_manager->key)) {
        key_manager->val = true;
        key_manager->time = 0;
        return;
    }

    if(IsKeyDown(key_manager->key)) {
        key_manager->time += GetFrameTime();

        if(key_manager->time >= REPEAT_KEY_INITIAL_DELAY) {
            if(key_manager->time - REPEAT_KEY_INITIAL_DELAY >= REPEAT_KEY_TICK_RATE) {
                key_manager->time = REPEAT_KEY_INITIAL_DELAY;
                key_manager->val = true;
                return;
            }
        }
    } else {
        key_manager->time = 0;
    }

    key_manager->val = false;
}

void keys_manager_update()
{
    update_key(&keys_manager.backspace);
    update_key(&keys_manager.right_arrow);
    update_key(&keys_manager.left_arrow);
}

bool is_repeating_key_active(KeysManagerKeys key)
{
    switch(key) {
        case KEYSM_BACKSPACE: return keys_manager.backspace.val;
        case KEYSM_RIGHT_ARROW: return keys_manager.right_arrow.val;
        case KEYSM_LEFT_ARROW: return keys_manager.left_arrow.val;
        default: return false;
    }
}
