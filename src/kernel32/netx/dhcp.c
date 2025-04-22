#include <netx.h>

void eth_proc_dhcp(eth_t *eth, udp_t *udp, uint16_t length)
{
    dhcp_t *dhcp = (dhcp_t *)udp->payload;
    uint16_t dhcp_length = length - sizeof(eth_t) - sizeof(ipv4_t) - sizeof(udp_t);
    if (dhcp->op == 2) {
        // 获取DHCP的数据包类型
        dhcp_option_t *option = (dhcp_option_t *)dhcp->options;
        while (option->code != 0xFF) {
            if (option->code == 53) { // DHCP Message Type
                if (option->data[0] == 2) { // DHCP Offer
                    dhcp_request(dhcp);
                    return;
                } else if (option->data[0] == 5) { // DHCP ACK
                    dhcp_ack(dhcp);
                    return;
                }
            }
            option = (dhcp_option_t *)((uint8_t *)option + option->length + 2);
        }
    }
}

void dhcp_discover(dhcp_t *dhcp)
{
    // 申请缓冲区
    e1000_t *e1000 = get_e1000dev();
    desc_buff_t *buff = alloc_desc_buff(e1000);
    // 构建数据包
    eth_t *eth = (eth_t *)buff->payload;
    eth_request(e1000, buff, "\xFF\xFF\xFF\xFF\xFF\xFF", ETH_TYPE_IPv4);
    buff->length += sizeof(eth_t);
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    ipv4_request(e1000, eth, "\x00\x00\x00\x00", IP_TYPE_ICMP, NULL, 0, NULL, 0);
    buff->length += sizeof(ipv4_t);
    udp_t *udp = (udp_t *)ipv4->payload;
    buff->length += sizeof(udp_t);
    // 构建DHCP数据包
    dhcp_t *dhcp = (dhcp_t *)udp->payload;
    // TODO
    buff->length += sizeof(dhcp_t);
    dhcp_option_t *option = (dhcp_option_t *)dhcp->options;
    // DHCP Message Type: Discover
    dhcp_option_t *opmt = (dhcp_option_t *)((uint8_t *)option);
    opmt->code = DHCP_OPTION_MESSAGE_TYPE;
    opmt->length = 1;
    opmt->data[0] = 1; // Discover
    buff->length += sizeof(dhcp_option_t) + opmt->length;
    // Client identifier
    dhcp_option_t *opci = (dhcp_option_t *)((uint8_t *)opmt + sizeof(dhcp_option_t) + opmt->length);
    opci->code = DNCP_OPTION_CLIENT_IDENTIFIER;
    opci->length = 19;
    // TODO
    buff->length += sizeof(dhcp_option_t) + opci->length;
    // Parameter Request List
    dhcp_option_t *opprl = (dhcp_option_t *)((uint8_t *)opci + sizeof(dhcp_option_t) + opci->length);
    opprl = DHCP_OPTION_PARAM_REQUEST_LIST;
    opprl->length = 11;
    // TODO
    buff->length += sizeof(dhcp_option_t) + opprl->length;
    // Maximum DHCP Message Size
    dhcp_option_t *opmms = (dhcp_option_t *)((uint8_t *)opprl + sizeof(dhcp_option_t) + opprl->length);
    opmms->code = DHCP_OPTION_MAX_MESSAGE_SIZE;
    opmms->length = 2;
    // TODO
    buff->length += sizeof(dhcp_option_t) + opmms->length;
    // Host name
    dhcp_option_t *ophm = (dhcp_option_t *)((uint8_t *)opmms + sizeof(dhcp_option_t) + opmms->length);
    ophm->code = DHCP_OPTION_HOST_NAME;
    // TODO
    ophm->length = 0;
    // TODO
    buff->length += sizeof(dhcp_option_t) + ophm->length;
    // End Option
    dhcp_option_t *opend = (dhcp_option_t *)((uint8_t *)ophm + sizeof(dhcp_option_t) + ophm->length);
    opend->code = DNCP_OPTION_END;
    buff->length += sizeof(dhcp_option_t) + ophm->length;
    udp_request(eth, PORT_DHCP_CLIENT, PORT_DHCP_SERVER, NULL, buff->length - sizeof(eth_t) - sizeof(ipv4_t));
    // 更新IPv4总长度
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    ipv4->total_len = htons(buff->length - sizeof(eth_t));
    // 发送数据包
    e1000_send_packet(buff);
    // 释放缓冲区
    free_desc_buff(e1000, buff);
}

void dhcp_request()
{
    // 申请缓冲区
    e1000_t *e1000 = get_e1000dev();
    desc_buff_t *buff = alloc_desc_buff(e1000);
    // 构建数据包
    eth_t *eth = (eth_t *)buff->payload;
    eth_request(e1000, buff, "\xFF\xFF\xFF\xFF\xFF\xFF", ETH_TYPE_IPv4);
    buff->length += sizeof(eth_t);
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    ipv4_request(e1000, eth, "\x00\x00\x00\x00", IP_TYPE_ICMP, NULL, 0, NULL, 0);
    buff->length += sizeof(ipv4_t);
    udp_t *udp = (udp_t *)ipv4->payload;
    buff->length += sizeof(udp_t);
    // 构建DHCP数据包
    dhcp_t *dhcp = (dhcp_t *)udp->payload;
    // TODO
    buff->length += sizeof(dhcp_t);
    dhcp_option_t *option = (dhcp_option_t *)dhcp->options;
    // DHCP Message Type: Discover
    dhcp_option_t *opmt = (dhcp_option_t *)((uint8_t *)option);
    opmt->code = DHCP_OPTION_MESSAGE_TYPE;
    opmt->length = 1;
    opmt->data[0] = 1; // Discover
    buff->length += sizeof(dhcp_option_t) + opmt->length;
    // Client identifier
    dhcp_option_t *opci = (dhcp_option_t *)((uint8_t *)opmt + sizeof(dhcp_option_t) + opmt->length);
    opci->code = DNCP_OPTION_CLIENT_IDENTIFIER;
    opci->length = 19;
    // TODO
    buff->length += sizeof(dhcp_option_t) + opci->length;
    // Parameter Request List
    dhcp_option_t *opprl = (dhcp_option_t *)((uint8_t *)opci + sizeof(dhcp_option_t) + opci->length);
    opprl = DHCP_OPTION_PARAM_REQUEST_LIST;
    opprl->length = 11;
    // TODO
    buff->length += sizeof(dhcp_option_t) + opprl->length;
    // Maximum DHCP Message Size
    dhcp_option_t *opmms = (dhcp_option_t *)((uint8_t *)opprl + sizeof(dhcp_option_t) + opprl->length);
    opmms->code = DHCP_OPTION_MAX_MESSAGE_SIZE;
    opmms->length = 2;
    // TODO
    buff->length += sizeof(dhcp_option_t) + opmms->length;
    // DHCP Server Identifier
    dhcp_option_t *opsi = (dhcp_option_t *)((uint8_t *)opmms + sizeof(dhcp_option_t) + opmms->length);
    opsi->code = DHCP_OPTION_SERVER_ID;
    opsi->length = 4;
    // TODO
    buff->length += sizeof(dhcp_option_t) + opsi->length;
    // Requested IP Address
    dhcp_option_t *opip = (dhcp_option_t *)((uint8_t *)opsi + sizeof(dhcp_option_t) + opsi->length);
    opsi->code = DHCP_OPTION_SERVER_ID;
    opsi->length = 4;
    // TODO
    buff->length += sizeof(dhcp_option_t) + opip->length;
    // Host name
    dhcp_option_t *ophm = (dhcp_option_t *)((uint8_t *)opip + sizeof(dhcp_option_t) + opip->length);
    ophm->code = DHCP_OPTION_HOST_NAME;
    // TODO
    ophm->length = 0;
    // TODO
    buff->length += sizeof(dhcp_option_t) + ophm->length;
    // End Option
    dhcp_option_t *opend = (dhcp_option_t *)((uint8_t *)ophm + sizeof(dhcp_option_t) + ophm->length);
    opend->code = DNCP_OPTION_END;
    buff->length += sizeof(dhcp_option_t) + ophm->length;
    udp_request(eth, PORT_DHCP_CLIENT, PORT_DHCP_SERVER, NULL, buff->length - sizeof(eth_t) - sizeof(ipv4_t));
    // 更新IPv4总长度
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    ipv4->total_len = htons(buff->length - sizeof(eth_t));
    // 发送数据包
    e1000_send_packet(buff);
    // 释放缓冲区
    free_desc_buff(e1000, buff);
}

void dhcp_offer(dhcp_t *dhcp)
{

}

void dhcp_ack(dhcp_t *dhcp)
{

}