#include <netx.h>

void eth_proc_icmp(eth_t *eth, uint16_t length)
{
    e1000_t *e1000 = get_e1000dev();
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    icmp_t *icmp = (icmp_t *)ipv4->payload;
    if (icmp->type == ICMP_TYPE_ECHO_REQUEST) {
        desc_buff_t *buff = alloc_desc_buff(e1000);
        buff->length = length;
        // 复制数据包
        kernel_memcpy(buff->payload, eth, length);
        // 构建应答包
        eth_reply(e1000, buff, eth);
        ipv4_replay(e1000, (eth_t *)buff->payload, NULL, 0, NULL, 0);
        icmp_reply(e1000, (eth_t *)buff->payload, NULL, 0);
        // 发送ICMP应答
        e1000_send_packet(buff);
        // 释放缓冲区
        free_desc_buff(e1000, buff);
    }
}

void icmp_request(e1000_t *e1000, eth_t *eth, uint8_t *data, uint16_t dlen)
{
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    icmp_t *icmp = (icmp_t *)ipv4->payload;
    icmp->type = ICMP_TYPE_ECHO_REQUEST; // 类型
    icmp->code = 0; // 代码
    icmp->checksum = 0; // 校验和
    if (data) 
        kernel_memcpy(icmp->payload, data, dlen); // 数据负载
    icmp->checksum = calc_checksum((uint8_t *)icmp, dlen + sizeof(icmp_t)); // 计算校验和
}

void icmp_reply(e1000_t *e1000, eth_t *eth, uint8_t *data, uint16_t dlen)
{
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    icmp_t *icmp = (icmp_t *)ipv4->payload;
    icmp->type = ICMP_TYPE_ECHO_REPLY; // 类型
    icmp->code = 0; // 代码
    icmp->checksum = 0; // 校验和
    if (data) 
        kernel_memcpy(icmp->payload, data, dlen); // 数据负载
    icmp->checksum = calc_checksum((uint8_t *)icmp, dlen + sizeof(icmp_t)); // 计算校验和
}