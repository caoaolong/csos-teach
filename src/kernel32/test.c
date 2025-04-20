#include <kernel.h>
#include <logf.h>
#include <netx.h>
#include <pci/e1000.h>
#include <csos/string.h>

void sys_test()
{
    // 申请缓冲区
    e1000_t *e1000 = get_e1000dev();
    desc_buff_t *buff = alloc_desc_buff(e1000);
    // 构建数据包
    eth_request(e1000, buff, "\xFF\xFF\xFF\xFF\xFF\xFF", ETH_TYPE_ARP);
    buff->length += sizeof(eth_t);
    eth_t *eth = (eth_t *)buff->payload;
    ip_addr dst_ip;
    inet_pton("192.168.10.21", dst_ip);
    arp_request(e1000, eth, dst_ip);
    buff->length += sizeof(arp_t);
    // 发送数据包
    e1000_send_packet(buff);
    // 释放缓冲区
    free_desc_buff(e1000, buff);
}