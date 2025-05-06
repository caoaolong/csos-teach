#include <netx.h>
#include <netx/eth.h>
#include <netx/ipv4.h>
#include <netx/udp.h>
#include <netx/dhcp.h>
#include <csos/string.h>

void dhcp_input(netif_t *netif, desc_buff_t *buff)
{
    eth_t *eth = (eth_t *)buff->payload;
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    udp_t *udp = (udp_t *)ipv4->payload;
    dhcp_t *dhcp = (dhcp_t *)udp->payload;
    if (dhcp->op == 2) {
        // 获取DHCP的数据包类型
        dhcp_option_t *option = (dhcp_option_t *)dhcp->options;
        while (option->code != DHCP_OPTION_END) {
            if (option->code == DHCP_OPTION_MESSAGE_TYPE) { // DHCP Message Type
                uint8_t *type = (uint8_t *)option->data;
                if (*type == DHCP_TYPE_OFFER) { // DHCP Offer
                    dhcp_offer(netif, buff);
                    dhcp_request(netif, buff);
                    return;
                } else if (*type == DHCP_TYPE_ACK) { // DHCP ACK
                    dhcp_ack(netif, buff);
                    return;
                }
            }
            option = (dhcp_option_t *)((uint8_t *)option + option->length + 2);
        }
    }
}

void dhcp_output(netif_t *netif, desc_buff_t *buff)
{
    
}

void dhcp_discover(netif_t *netif, desc_buff_t *buff)
{
    eth_t *eth = (eth_t *)buff->payload;
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    udp_t *udp = (udp_t *)ipv4->payload;
    dhcp_t *dhcp = (dhcp_t *)udp->payload;
    dhcp->op = 1;
    dhcp->htype = 1; // Ethernet
    dhcp->hlen = 6; // MAC地址长度
    dhcp->hops = 0; // Hops
    dhcp->xid = htonl(xrandom()); // 事务ID
    dhcp->secs = htons(1); // Seconds elapsed
    dhcp->flags = 0; // Flags
    kernel_memset(dhcp->ciaddr, 0, IPV4_LEN); // Client IP address
    kernel_memset(dhcp->yiaddr, 0, IPV4_LEN); // Your IP address
    kernel_memset(dhcp->siaddr, 0, IPV4_LEN); // Server IP address
    kernel_memset(dhcp->giaddr, 0, IPV4_LEN); // Gateway IP address
    kernel_memset(dhcp->chaddr, 0, sizeof(dhcp->chaddr)); // Client hardware address
    kernel_memcpy(dhcp->chaddr, netif->mac, MAC_LEN); // Client hardware address
    kernel_memset(dhcp->sname, 0, sizeof(dhcp->sname)); // Server host name
    kernel_memset(dhcp->file, 0, sizeof(dhcp->file)); // Boot file name
    kernel_memcpy(dhcp->magic, (void *)DHCP_MAGIC_COOKIE, sizeof(dhcp->magic)); // Magic cookie
    uint16_t opl = 0;
    dhcp_option_t *option = (dhcp_option_t *)dhcp->options;
    // DHCP Message Type: Discover
    dhcp_option_t *opmt = (dhcp_option_t *)((uint8_t *)option);
    opmt->code = DHCP_OPTION_MESSAGE_TYPE;
    opmt->length = 1;
    opmt->data[0] = 1;
    opl += sizeof(dhcp_option_t) + opmt->length;
    // Parameter Request List
    dhcp_option_t *opprl = (dhcp_option_t *)((uint8_t *)opmt + sizeof(dhcp_option_t) + opmt->length);
    opprl->code = DHCP_OPTION_PARAM_REQUEST_LIST;
    opprl->length = 5;
    kernel_memcpy(opprl->data, (void *)DHCP_PARAM_REQUEST_LIST, opprl->length);
    opl += sizeof(dhcp_option_t) + opprl->length;
    // End Option
    dhcp_option_t *opend = (dhcp_option_t *)((uint8_t *)opprl + sizeof(dhcp_option_t) + opprl->length);
    opend->code = DHCP_OPTION_END;
    opl++;
    udp_build(netif, buff, "\xFF\xFF\xFF\xFF", DHCP_CLIENT_PORT, DHCP_SERVER_PORT, NULL, sizeof(dhcp_t) + opl);
}

void dhcp_request(netif_t *netif, desc_buff_t *buff)
{
    eth_t *eth = (eth_t *)buff->payload;
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    udp_t *udp = (udp_t *)ipv4->payload;
    dhcp_t *dhcp = (dhcp_t *)udp->payload;
    dhcp->op = 1;
    uint16_t opl = 0;
    dhcp_option_t *option = (dhcp_option_t *)dhcp->options;
    // DHCP Message Type: Request
    dhcp_option_t *opmt = (dhcp_option_t *)((uint8_t *)option);
    opmt->length = 1;
    opmt->data[0] = 3;
    opl += sizeof(dhcp_option_t) + opmt->length;
    // Parameter Request List
    dhcp_option_t *opprl = (dhcp_option_t *)((uint8_t *)opmt + sizeof(dhcp_option_t) + opmt->length);
    opprl->code = DHCP_OPTION_PARAM_REQUEST_LIST;
    opprl->length = 5;
    kernel_memcpy(opprl->data, (void *)DHCP_PARAM_REQUEST_LIST, opprl->length);
    opl += sizeof(dhcp_option_t) + opprl->length;
    // Request IP Address
    dhcp_option_t *oprip = (dhcp_option_t *)((uint8_t *)opprl + sizeof(dhcp_option_t) + opprl->length);
    oprip->code = DHCP_OPTION_REQUESTED_IP;
    oprip->length = 4;
    kernel_memcpy(oprip->data, netif->dhcp_ipv4, IPV4_LEN);
    opl += sizeof(dhcp_option_t) + oprip->length;
    // End Option
    dhcp_option_t *opend = (dhcp_option_t *)((uint8_t *)oprip + sizeof(dhcp_option_t) + oprip->length);
    opend->code = DHCP_OPTION_END;
    opl++;
    udp_build(netif, buff, "\xFF\xFF\xFF\xFF", DHCP_CLIENT_PORT, DHCP_SERVER_PORT, NULL, sizeof(dhcp_t) + opl);
}

void dhcp_offer(netif_t *netif, desc_buff_t *buff)
{
    eth_t *eth = (eth_t *)buff->payload;
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    udp_t *udp = (udp_t *)ipv4->payload;
    dhcp_t *dhcp = (dhcp_t *)udp->payload;
    kernel_memcpy(netif->dhcp_ipv4, dhcp->yiaddr, IPV4_LEN);
    dhcp_option_t *option = (dhcp_option_t *)dhcp->options;
    while (option->code != DHCP_OPTION_END) {
        uint8_t len = option->length;
        if (option->code == DHCP_OPTION_ROUTER) {
            kernel_memcpy(netif->dhcp_gw, option->data, IPV4_LEN);
        } else if (option->code == DHCP_OPTION_SUBNET_MASK) {
            kernel_memcpy(netif->dhcp_mask, option->data, IPV4_LEN);
        }
        option = (dhcp_option_t *)(((uint8_t *)option) + sizeof(dhcp_option_t) + len);
    }
    netif->status = NETIF_STATUS_REQUESTED;
}

void dhcp_ack(netif_t *netif, desc_buff_t *buff)
{
    kernel_memcpy(netif->gw, netif->dhcp_gw, IPV4_LEN);
    kernel_memcpy(netif->mask, netif->dhcp_mask, IPV4_LEN);
    kernel_memcpy(netif->ipv4, netif->dhcp_ipv4, IPV4_LEN);
    netif->status = NETIF_STATUS_ACK;
    net_save();
}