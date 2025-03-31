#include <list.h>

void list_init(list_t *list)
{
    list->head = list->tail = NULL;
    list->size = 0;
}

// type=0: 往后插入; type=1: 往前插入
void list_insert_node(list_t *list, list_node_t *nnode, list_node_t *onode, int type)
{
    // 参数检查
    if (!list || !nnode || !onode) {
        return;
    }

    if (type == 0) {  // 向后插入
        if (!onode->next) {
            list->tail = nnode;
        } else {
            onode->next->pre = nnode;  // 更新tmp的pre指针
        }
        list_node_t *tmp = onode->next;
        onode->next = nnode;
        nnode->next = tmp;
        nnode->pre = onode;
    } else if (type == 1) {  // 向前插入
        if (!onode->pre) {
            list->head = nnode;
        } else {
            onode->pre->next = nnode;  // 更新tmp的next指针
        }
        list_node_t *tmp = onode->pre;
        onode->pre = nnode;
        nnode->pre = tmp;
        nnode->next = onode;
    } else {
        return;  // 非法的type值
    }
    list->size++;
}

void list_insert_front(list_t *list, list_node_t *node)
{
    node->next = list->head;
    node->pre = NULL;
    if (list_is_empty(list))
        list->tail = list->head = node;
    else {
        list->head->pre = node;
        list->head = node;
    }
    list->size ++;
}

void list_insert_back(list_t *list, list_node_t *node)
{
    node->pre = list->tail;
    node->next = NULL;
    if (list_is_empty(list))
        list->head = list->tail = node;
    else {
        list->tail->next = node;
        list->tail = node;
    }
    list->size ++;
}

list_node_t *list_remove(list_t *list, list_node_t *node)
{
    if (list->head == node) {
        list->head = node->next;
    }

    if (list->tail == node) {
        list->tail = node->pre;
    }

    if (node->pre) {
        node->pre->next = node->next;
    }

    if (node->next) {
        node->next->pre = node->pre;
    }

    node->pre = node->next = NULL;
    list->size --;
    return node;
}

list_node_t *list_remove_front(list_t *list)
{
    if (list_is_empty(list))
        return NULL;
    
    list_node_t *remove_node = list->head;
    list->head = remove_node->next;
    if (list->head == NULL) {
        list->tail = NULL;
    } else {
        remove_node->next->pre = NULL;
    }
    remove_node->pre = remove_node->next = NULL;
    list->size --;
    return remove_node;
}

list_node_t *list_get_first(list_t *list)
{
    return list->head;
}

list_node_t *list_get_last(list_t *list)
{
    return list->tail;
}

list_node_t *list_get_pre(list_node_t *node)
{
    return node->pre;
}

list_node_t *list_get_next(list_node_t *node)
{
    return node->next;
}

void test_list()
{
    list_t list;
    list_init(&list);

    list_node_t nodes[6];
    list_node_t *node = NULL;
    for (int i = 0; i < 5; i++)
    {
        node = &nodes[i];
        list_insert_front(&list, node);
    }
    list_insert_back(&list, &node[5]);

    list_remove(&list, &nodes[2]);
    list_remove_front(&list);

    for (int i = 0; i < 4; i++)
    {
        list_node_t *node = list_remove_front(&list);
    }
    
    // 测试通过字段指针获得结构体指针
    struct test_t
    {
        int i;
        list_node_t node;
    } v = { 0x123 };
    list_node_t *v_node = &v.node;
    uint32_t offset = field_offset(struct test_t, node);
    struct test_t *r = struct_from_field(v_node, struct test_t, node);
}