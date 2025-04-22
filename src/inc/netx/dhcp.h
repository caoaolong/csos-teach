#ifndef CSOS_DHCP_H
#define CSOS_DHCP_H

#include <types.h>
#include <netx/eth.h>

#define DNCP_OPTION_END                 255
#define DNCP_OPTION_CLIENT_IDENTIFIER   61
#define DHCP_OPTION_MAX_MESSAGE_SIZE    57
#define DHCP_OPTION_PARAM_REQUEST_LIST  55
#define DHCP_OPTION_MESSAGE_TYPE        53
#define DHCP_OPTION_SERVER_ID           54
#define DHCP_OPTION_REQUESTED_IP        50
#define DHCP_OPTION_LEASE_TIME          51
#define DHCP_OPTION_HOST_NAME           12
#define DHCP_OPTION_SUBNET_MASK         1
#define DHCP_OPTION_ROUTER              3
#define DHCP_OPTION_DNS                 6

typedef struct dhcp_t {
    uint8_t op;          // 1: BOOTREQUEST, 2: BOOTREPLY
    uint8_t htype;      // 1: Ethernet
    uint8_t hlen;       // Hardware address length
    uint8_t hops;       // Number of hops
    uint32_t xid;       // Transaction ID
    uint16_t secs;      // Seconds elapsed since client began address acquisition or renewal process
    uint16_t flags;     // Flags
    ip_addr ciaddr;     // Client IP address (if already in use)
    ip_addr yiaddr;     // Your IP address (client IP address assigned by server)
    ip_addr siaddr;     // Server IP address (optional)
    ip_addr giaddr;     // Gateway IP address (optional)
    mac_addr chaddr;    // Client hardware address (MAC address)
    char sname[64];     // Server host name (optional)
    char file[128];     // Boot file name (optional)
    uint8_t options[0]; // Options field (variable length)
} dhcp_t;

typedef struct dhcp_option_t {
    uint8_t code;       // Option code
    uint8_t length;     // Length of option data
    uint8_t data[0];    // Option data (variable length)
} dhcp_option_t;

void eth_proc_dhcp(eth_t *eth, udp_t *udp, uint16_t length);

void dhcp_discover();
void dhcp_offer(dhcp_t *dhcp);
void dhcp_request(dhcp_t *dhcp);
void dhcp_ack(dhcp_t *dhcp);

#endif