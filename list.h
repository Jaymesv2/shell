#ifndef _LIST_H_
#define _LIST_H_
#include "common.h"
#include <string.h>

typedef struct list_node {
    void *elem;
    struct list_node *next;
} list_node_t;

typedef struct list {
    list_node_t *_Nullable head;
    list_node_t *_Nonnull *_Nullable tail;
} list_t;

list_t *new_list();

int peek_front(list_t *_Nonnull lst, void **_Nonnull elem);

int _list_length(list_node_t *_Nonnull lst);

int list_length(list_t *_Nonnull lst);

void push_front(list_t *_Nonnull lst, void *_Nullable elem);

void* pop_front(list_t *_Nonnull lst);

void push_back(list_t *_Nonnull lst, void *_Nullable elem);

void drop_list(list_t *_Nonnull lst, void (*_Nullable free_node)(void*));

#endif