#include <netx.h>
#include <csos/string.h>

void eth_proc_ipv4(eth_t *eth, uint16_t length)
{
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    switch (ipv4->proto)
    {
    case IP_TYPE_ICMP:
        eth_proc_icmp(eth, length);
        break;
    case IP_TYPE_TCP:
        /* code */
        break;
    case IP_TYPE_UDP:
        /* code */
        break;
    default:
        break;
    }
}

void ipv4_request(
    e1000_t *e1000, eth_t *eth, ip_addr src, ip_addr dst, uint8_t proto, 
    uint8_t *options, uint16_t oplen, 
    uint8_t *data, uint16_t dlen)
{
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    ipv4->ver = 0x4; // 版本号和头部长度
    ipv4->ihl = 0x5 + oplen / 4; // 头部长度
    ipv4->tos = 0; // 服务类型
    ipv4->total_len = htons(sizeof(ipv4_t) + oplen + dlen); // 总长度
    ipv4->id = (uint16_t)xrandom();
    // TODO: 根据数据长度设置
    ipv4->flags = 0;
    ipv4->ttl = 64;
    ipv4->proto = proto;
    kernel_memcpy(ipv4->src_ip, src, IPV4_LEN);
    kernel_memcpy(ipv4->dst_ip, dst, IPV4_LEN);
    // TODO: 处理Options和Paddings
    ipv4->checksum = 0;
    ipv4->checksum = calc_checksum((uint8_t *)ipv4, sizeof(ipv4_t));
    if(data) 
        kernel_memcpy(ipv4->payload, data, dlen);
}

void ipv4_request(
    e1000_t *e1000, eth_t *eth, ip_addr dst, uint8_t proto, 
    uint8_t *options, uint16_t oplen, 
    uint8_t *data, uint16_t dlen)
{
    ipv4_request(e1000, eth, e1000->ipv4, dst, proto, options, oplen, data, dlen);
}

void ipv4_replay(
    e1000_t *e1000, eth_t *eth, 
    uint8_t *options, uint16_t oplen,
    uint8_t *data, uint16_t dlen)
{
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    // 保存发送地址
    ip_addr target_ip;
    kernel_memcpy(target_ip, ipv4->src_ip, IPV4_LEN);
    // 构建数据包
    ipv4->id = (uint16_t)xrandom();
    // TODO: 根据数据长度设置
    ipv4->flags = 0;
    kernel_memcpy(ipv4->src_ip, e1000->ipv4, IPV4_LEN);
    kernel_memcpy(ipv4->dst_ip, target_ip, IPV4_LEN);
    // TODO: 处理Options和Paddings
    ipv4->checksum = 0;
    ipv4->checksum = calc_checksum((uint8_t *)ipv4, sizeof(ipv4_t));
    if(data) 
        kernel_memcpy(ipv4->payload, data, dlen);
}