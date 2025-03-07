#include <string.h>

#include "cTooling.h"

String string_create(const char *text)
{
    String str = {0};

    if(text != NULL) {
        da_append_many(&str, text, strlen(text));
    }

    return str;
}

void string_append_text(String *str, const char *text)
{
    da_append_many(str, text, strlen(text));
}

void string_append_chr(String *str, char c)
{
    da_append(str, c);
}

void string_append_string(String *dest, String src)
{
    for(size_t i = 0; i < src.count; i++) {
        da_append(dest, src.items[i]);
    }
}

void string_insert_text(String *str, const char *text, size_t pos)
{
    if(pos > str->count) pos = str->count;

    size_t len = strlen(text);

    if(len + str->count > str->capacity) {
        str->capacity = len + str->count;
        str->items = realloc(str->items, str->capacity);

        assert(str->items != NULL && "No enough ram");
    }

    memmove(str->items + pos + len, str->items + pos, str->count - pos);
    memcpy(str->items + pos, text, len);

    str->count += len;
}
void string_insert_chr(String *str, char c, size_t pos)
{
    if(pos > str->count) pos = str->count;

    size_t count = str->count;

    da_append(str, '\0');

    memmove(str->items + pos + 1, str->items + pos, count - pos);
    str->items[pos] = c;
}

void string_remove_chr(String *str, size_t pos)
{
    if(pos > str->count - 1) return;

    for(size_t i = pos; i < str->count - 1; i++) {
        str->items[i] = str->items[i + 1];
    }

    str->count--;
}

void string_remove_slice(String *str, size_t start, size_t end)
{
    if(end > str->count) {
        end = str->count;
    }

    if(start >= end) return;

    memmove(str->items + start, str->items + end, str->count - end);
    str->count -= end - start;
}

void string_free(String *str)
{
    da_free(str);
}

LList *llist_create()
{
    LList *list = malloc(sizeof(LList));
    bzero(list, sizeof(LList));
    return list;
}

void llist_append_node(LList *list, LNode *node)
{
    if(list->count == 0) {
        list->head = node;
        list->tail = node;
    } else {
        list->tail->next = node;
        list->tail = node;
    }

    list->count++;
}

void llist_destroy(LList *list)
{
    LNode *node = list->head;

    while(node != NULL) {
        LNode *_node = node;
        node = node->next;
        free(_node);
    }

    free(list);
}

LNode *llist_create_node(int type, void *data)
{
    LNode *node = malloc(sizeof(LNode));
    node->type = type;
    node->data = data;
    node->next = NULL;
    return node;
}
