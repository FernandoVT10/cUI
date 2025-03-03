#ifndef KEYS_MANAGER_H
#define KEYS_MANAGER_H

typedef enum {
    KEYSM_BACKSPACE,
    KEYSM_RIGHT_ARROW,
    KEYSM_LEFT_ARROW,
} KeysManagerKeys;

void keys_manager_update();

bool is_repeating_key_active(KeysManagerKeys key);

#endif // KEYS_MANAGER_H
