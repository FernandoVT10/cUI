#ifndef CTOOLING_H
#define CTOOLING_H

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define DA_INIT_CAP 128

#define da_append(da, item)                                                          \
    do {                                                                             \
        if((da)->count >= (da)->capacity) {                                          \
            (da)->capacity = (da)->capacity == 0 ? DA_INIT_CAP : (da)->capacity*2;   \
            (da)->items = realloc((da)->items, (da)->capacity*sizeof(*(da)->items)); \
            assert((da)->items != NULL && "No enough ram");                          \
        }                                                                            \
                                                                                     \
        (da)->items[(da)->count++] = (item);                                         \
    } while(0)

#define da_free(da) do { free((da)->items); } while(0)

#define da_append_many(da, new_items, new_items_count)                                  \
    do {                                                                                    \
        if ((da)->count + (new_items_count) > (da)->capacity) {                               \
            if ((da)->capacity == 0) {                                                      \
                (da)->capacity = DA_INIT_CAP;                                           \
            }                                                                               \
            while ((da)->count + (new_items_count) > (da)->capacity) {                        \
                (da)->capacity *= 2;                                                        \
            }                                                                               \
            (da)->items = realloc((da)->items, (da)->capacity*sizeof(*(da)->items)); \
            assert((da)->items != NULL && "No enough ram");                          \
        }                                                                                   \
        memcpy((da)->items + (da)->count, (new_items), (new_items_count)*sizeof(*(da)->items)); \
        (da)->count += (new_items_count);                                                     \
    } while (0)

typedef struct {
    char *items;
    size_t count;
    size_t capacity;
} String;

// String Manipulation Functions
String string_create(const char *text);
void string_append_text(String *str, const char *text);
void string_append_chr(String *str, char c);
void string_append_string(String *dest, String src);
void string_insert_text(String *str, const char *text, size_t pos);
void string_insert_chr(String *str, char c, size_t pos);
void string_remove_chr(String *str, size_t pos);
void string_remove_slice(String *str, size_t start, size_t end);
void string_free(String *str);

typedef struct LNode LNode;

typedef struct {
    LNode *head;
    LNode *tail;
    size_t count;
} LList;

struct LNode {
    LNode *next;
    void *data;
    int type;
};

// Linked List functions
LList *llist_create();
void llist_append_node(LList *list, LNode *node);
void llist_destroy(LList *list);

// Linked list node functions
LNode *llist_create_node(int type, void *data);

#endif // CTOOLING_H
