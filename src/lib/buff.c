#include <paging.h>
#include <pci/e1000.h>
#include <csos/memory.h>

static list_t buff_list;

void free_desc_buff(desc_buff_t *buff)
{
    list_t *list = &buff_list;
    list_node_t *node = list_remove(list, &buff->node);
    if (list->size % 2 == 0) {
        free_page((uint32_t)node & ~0xFFF);
    }
}

desc_buff_t *alloc_desc_buff()
{
    list_t *list = &buff_list;
    desc_buff_t *buf = NULL;
    if (list->size % 2 == 0) {
        buf = (desc_buff_t *)alloc_page();
    } else {
        list_node_t *last = list_get_last(list);
        uint8_t *pbuf = (uint8_t *)struct_from_field(last, desc_buff_t, node);
        buf = (desc_buff_t *)pbuf + (PAGE_SIZE / 2);
    }
    buf->length = 0;
    buf->refc = 0;
    list_node_init(&buf->node);
    list_insert_back(list, &buf->node);
    return buf;
}

void desc_buff_init()
{
    list_init(&buff_list);
}