#include <stdlib.h>
#include "list.h"

list_t *new_list() {
    list_t *lst = malloc(sizeof(list_t));
    lst->head = NULL;
    lst->tail = &(lst->head);
    return lst;
}

int peek_front(list_t *_Nonnull lst, void **_Nonnull elem) {
    if(lst->head == NULL) 
        return 0;
    *elem = lst->head->elem;
    return 1;
}

int _list_length(list_node_t *_Nonnull lst) {
    return lst == NULL ? 0 : _list_length(lst->next)+1;
}

int list_length(list_t *_Nonnull lst) {
    return _list_length(lst->head);   
}

void push_front(list_t *_Nonnull lst, void *_Nullable elem) {
    list_node_t *tmp = malloc(sizeof(list_node_t));
    if(tmp == NULL) {
        elog("failed to allocate a node, returning");
        return;
    }
    
    tmp->elem = elem;
    
    tmp->next = lst->head;
    lst->head = tmp;
}

void* pop_front(list_t *_Nonnull lst) {
    if(lst->head == NULL)
        return NULL;

    list_node_t *tmp = lst->head;
    
    lst->head = tmp->next;

    if(lst->head == NULL) 
        lst->tail = &(lst->head);
    else 
        lst->tail = &(lst->head->next);

    void* elem = tmp -> elem;
    free(tmp);
    return elem;
}

void push_back(list_t *_Nonnull lst, void *_Nullable elem) {
    list_node_t *tmp = malloc(sizeof(list_node_t));
    if(tmp == NULL) {
        elog("failed to allocate a node, returning");
        return;
    }
    tmp->elem = elem;
    tmp->next = NULL;

    *(lst->tail) = tmp;
    lst->tail = &(tmp->next);
    return;    
}

void drop_list(list_t *_Nonnull lst, void (*_Nullable free_node)(void*)) {
    list_node_t*tmp, *cur = lst->head;
    while(cur != NULL) {
        if(free_node != NULL)
            free_node(cur->elem);
        tmp = cur->next;
        free(cur);
        cur = tmp;        
    }
}