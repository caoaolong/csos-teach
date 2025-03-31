#ifndef CSOS_LIST_H
#define CSOS_LIST_H

#include <kernel.h>

typedef struct list_node_t
{
    struct list_node_t *pre;
    struct list_node_t *next;
} list_node_t;

typedef struct list_t
{
    list_node_t *head;
    list_node_t *tail;
    uint32_t size;
} list_t;

void list_init(list_t *list);

static inline void  list_node_init(list_node_t *node)
{
    node->pre = node->next = NULL;
}

static inline BOOL list_is_empty(list_t *list)
{
    return list->size == 0;
}

void list_insert_node(list_t *list, list_node_t *nnode, list_node_t *onode, int type);

void list_insert_front(list_t *list, list_node_t *node);

void list_insert_back(list_t *list, list_node_t *node);

list_node_t *list_remove(list_t *list, list_node_t *node);

list_node_t *list_remove_front(list_t *list);

list_node_t *list_get_first(list_t *list);

list_node_t *list_get_last(list_t *list);

list_node_t *list_get_pre(list_node_t *node);

list_node_t *list_get_next(list_node_t *node);

#define field_offset(pType, fieldName) \
    ((uint32_t)&(((pType *)0)->fieldName))

#define struct_address(pNode, pType, fieldName) \
    ((uint32_t)pNode - field_offset(pType, fieldName))

#define struct_from_field(pNode, pType, fieldName) \
    ((pType *)(pNode ? struct_address(pNode, pType, fieldName) : 0))

void test_list();

#endif