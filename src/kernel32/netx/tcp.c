#include <logf.h>
#include <netx/tcp.h>
#include <netx/eth.h>
#include <netx/ipv4.h>
#include <csos/string.h>

uint16_t calc_tcp_checksum(ipv4_t *ip, tcp_t *tcp, uint8_t *payload, uint16_t data_len)
{
    uint32_t checksum = 0;
    // 从 tcp->offset 字段提取 TCP 头部长度（高 4 位）
    uint16_t tcp_hdr_len = ntohs(ip->total_len) - ip->ihl * 4 - data_len;
    uint16_t tcp_len = tcp_hdr_len + data_len;

    // 1. Pseudo header
    checksum += (ip->src_ip[0] << 8) | ip->src_ip[1];
    checksum += (ip->src_ip[2] << 8) | ip->src_ip[3];
    checksum += (ip->dst_ip[0] << 8) | ip->dst_ip[1];
    checksum += (ip->dst_ip[2] << 8) | ip->dst_ip[3];
    checksum += IP_TYPE_TCP;     // Protocol number (6), 8 bits
    checksum += tcp_len;         // TCP length (already in host order)

    // 2. TCP header
    tcp->checksum = 0;
    uint8_t *tcp_bytes = (uint8_t *)tcp;
    for (uint16_t i = 0; i < tcp_hdr_len; i += 2) {
        uint16_t word = tcp_bytes[i] << 8;
        if (i + 1 < tcp_hdr_len) {
            word |= tcp_bytes[i + 1];
        }
        checksum += word;
    }

    // 3. TCP data
    for (uint16_t i = 0; i < data_len; i += 2) {
        uint16_t word = payload[i] << 8;
        if (i + 1 < data_len) {
            word |= payload[i + 1];
        }
        checksum += word;
    }

    // 4. Fold high bits into low 16 bits
    while (checksum >> 16)
        checksum = (checksum & 0xFFFF) + (checksum >> 16);

    return htons(~checksum ? (uint16_t)(~checksum) : 0xFFFF);  // 0 is not allowed
}

void tcp_input(netif_t *netif, desc_buff_t *buff)
{
    eth_t *eth = (eth_t *)buff->payload;
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    tcp_t *tcp = (tcp_t *)ipv4->payload;
    port_t *port = get_port(ntohs(tcp->dst_port));
    tcp->ff.v = ntohs(tcp->ff.v);
    if (port && port->status == PORT_UP) {
        if (tcp->ff.flags == (FLAGS_SYN | FLAGS_ACK)) {
            tcp_ack(port->sock, buff);
        } else if (tcp->ff.flags == (FLAGS_FIN | FLAGS_ACK)) {
            socket_t *socket = port->sock;
            tcp_ack(port->sock, buff);
            socket->srcp = ntohs(tcp->dst_port);
            socket->dstp = ntohs(tcp->src_port);
            desc_buff_t *nbuff = alloc_desc_buff();
            kernel_memcpy(nbuff, buff, buff->length);
            tcp_finack(port->sock, nbuff, ipv4->dst_ip);
        } else if (tcp->ff.flags == (FLAGS_RST | FLAGS_ACK)) {
            socket_t *socket = port->sock;
            socket->state = TCP_CLOSED;
        } else if (tcp->ff.flags == (FLAGS_PSH | FLAGS_ACK)) {
            logf("[CLIENT RECV]: %s", tcp->payload);
            tcp_ack(port->sock, buff);
        } else if(tcp->ff.flags == FLAGS_ACK) {
            socket_t *socket = port->sock;
            // 处理ACK包
            if (socket->state == TCP_FIN_WAIT_1) {
                socket->state = TCP_FIN_WAIT_2;
                free_desc_buff(buff);
            } else if (socket->state == TCP_SYN_RECEIVED) {
                socket->state = TCP_ESTABLISHED;
            } else if (socket->state == TCP_ESTABLISHED) {
                socket->ack = ntohl(tcp->seq_num);
                socket->seq = ntohl(tcp->ack_num);
                reply_level3_desc_buff(port->sock, buff);
            }
        }
    } else if (port->status == PORT_LISTEN) {
        if (tcp->ff.flags == (FLAGS_FIN | FLAGS_ACK)) {
            socket_t *socket = port->sock;
            if (socket->state == TCP_ESTABLISHED) {
                tcp_ack(port->sock, buff);
                desc_buff_t *nbuff = alloc_desc_buff();
                kernel_memcpy(nbuff, buff, buff->length);
                tcp_finack(port->sock, nbuff, ipv4->dst_ip);
            } else if (socket->state == TCP_CLOSE_WAIT) {
                tcp_ack(port->sock, buff);
            } else {
                // TODO: 发送RST数据包
                free_desc_buff(buff);
            }
        } else if (tcp->ff.flags == (FLAGS_RST | FLAGS_ACK)) {
            socket_t *socket = port->sock;
            socket->state = TCP_CLOSED;
        } else if (tcp->ff.flags == (FLAGS_PSH | FLAGS_ACK)) {
            logf("[SERVER RECV]: %s", tcp->payload);
            tcp_ack(port->sock, buff);
        } else if(tcp->ff.flags == FLAGS_SYN) {
            tcp_synack(port->sock, buff);
        } else if(tcp->ff.flags == FLAGS_ACK) {
            socket_t *socket = port->sock;
            // 处理ACK包
            if (socket->state == TCP_FIN_WAIT_1) {
                socket->state = TCP_FIN_WAIT_2;
                free_desc_buff(buff);
            } else if (socket->state == TCP_SYN_RECEIVED) {
                socket->state = TCP_ESTABLISHED;
            } else if (socket->state == TCP_ESTABLISHED) {
                socket->ack = ntohl(tcp->seq_num);
                socket->seq = ntohl(tcp->ack_num);
            }
        }
    } else {
        // TODO: 发送RST数据包
        free_desc_buff(buff);
    }
}

void tcp_output(netif_t *netif, desc_buff_t *buff, uint8_t *data, uint16_t dlen)
{
    eth_t *eth = (eth_t *)buff->payload;
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    tcp_t *tcp = (tcp_t *)ipv4->payload;
}

void tcp_build(socket_t *socket, desc_buff_t *buff, uint8_t *data, uint16_t dlen)
{
    netif_t *netif = socket->netif;
    eth_t *eth = (eth_t *)buff->payload;
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    tcp_t *tcp = (tcp_t *)ipv4->payload;
    tcp->src_port = htons(socket->srcp);
    tcp->dst_port = htons(socket->dstp);

    tcp->seq_num = htonl(socket->seq);
    tcp->ack_num = htonl(socket->ack);
    
    tcp->ff.flags = FLAGS_ACK | FLAGS_PSH;
    tcp->ff.unused = 0;
    tcp->ff.offset = sizeof(tcp_t) / 4;
    tcp->ff.v = htons(tcp->ff.v);
    tcp->window_size = htons(0xFF);
    tcp->checksum = 0;
    tcp->urgent_pointer = 0;
    buff->length += sizeof(tcp_t);
    ipv4_build(netif, buff, socket->dipv4, IP_TYPE_TCP, data, dlen, NULL, 0);
}

void tcp_syn(socket_t *socket, desc_buff_t *buff, ip_addr dst_ip, uint16_t dst_port)
{
    netif_t *netif = socket->netif;
    eth_t *eth = (eth_t *)buff->payload;
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    tcp_t *tcp = (tcp_t *)ipv4->payload;
    socket->srcp = alloc_random_port(socket, DBT_TCP);
    tcp->src_port = htons(socket->srcp);
    tcp->dst_port = htons(socket->dstp);

    socket->seq = xrandom();
    tcp->seq_num = htonl(socket->seq);
    tcp->ack_num = 0;
    
    tcp->ff.flags = FLAGS_SYN;
    tcp->ff.unused = 0;
    tcp->ff.offset = sizeof(tcp_t) / 4;
    tcp->ff.v = htons(tcp->ff.v);
    tcp->window_size = htons(0xFF);
    tcp->checksum = 0;
    tcp->urgent_pointer = 0;
    buff->length += sizeof(tcp_t);
    ipv4_build(netif, buff, dst_ip, IP_TYPE_TCP, NULL, 0, NULL, 0);
    socket->state = TCP_SYN_SENT;
}

void tcp_synack(socket_t *socket, desc_buff_t *buff)
{
    netif_t *netif = socket->netif;
    eth_t *eth = (eth_t *)buff->payload;
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    tcp_t *tcp = (tcp_t *)ipv4->payload;
    // 接收到的数据包的src_port和dst_port需要交换
    uint16_t dst_port = tcp->src_port;
    tcp->src_port = tcp->dst_port;
    tcp->dst_port = dst_port;
    // 服务端
    socket->dstp = htons(dst_port);
    kernel_memcpy(socket->dipv4, ipv4->src_ip, IPV4_LEN);

    uint32_t seq_num = ntohl(tcp->seq_num);
    socket->seq = xrandom();
    socket->ack = seq_num + 1;
    tcp->seq_num = htonl(socket->seq);
    tcp->ack_num = htonl(socket->ack);
    socket->seq++;

    char options[] = {'\x02', '\x04', '\x05', '\xb4'};
    kernel_memcpy(tcp->payload, options, sizeof(options));

    tcp->ff.flags = FLAGS_SYN | FLAGS_ACK;
    tcp->ff.unused = 0;
    tcp->ff.offset = (sizeof(tcp_t) + sizeof(options)) / 4;
    tcp->ff.v = htons(tcp->ff.v);
    tcp->checksum = 0;
    buff->length = sizeof(tcp_t) + sizeof(options);

    ip_addr dst_ip;
    kernel_memcpy(dst_ip, ipv4->src_ip, IPV4_LEN);
    ipv4_build(socket->netif, buff, dst_ip, IP_TYPE_TCP, NULL, 0, options, sizeof(options));
    socket->state = TCP_SYN_RECEIVED;
}

void tcp_finack(socket_t *socket, desc_buff_t *buff, ip_addr dst_ip)
{
    buff->length = 0;

    netif_t *netif = socket->netif;
    eth_t *eth = (eth_t *)buff->payload;
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    tcp_t *tcp = (tcp_t *)ipv4->payload;
    tcp->src_port = htons(socket->srcp);
    tcp->dst_port = htons(socket->dstp);
    tcp->seq_num = htonl(socket->seq);
    tcp->ack_num = htonl(socket->ack);
    tcp->ff.flags = FLAGS_FIN | FLAGS_ACK;
    tcp->ff.unused = 0;
    tcp->ff.offset = sizeof(tcp_t) / 4;
    tcp->ff.v = htons(tcp->ff.v);
    tcp->window_size = htons(0xFF);
    tcp->checksum = 0;
    tcp->urgent_pointer = 0;
    buff->length += sizeof(tcp_t);
    ipv4_build(netif, buff, dst_ip, IP_TYPE_TCP, NULL, 0, NULL, 0);
    if (socket->socktype == SOCK_CLIENT) {
        if (socket->state == TCP_ESTABLISHED) {
            socket->state = TCP_FIN_WAIT_1;
        }
    } else if (socket->socktype == SOCK_SERVER) {
        if (socket->state == TCP_CLOSE_WAIT) {
            socket->state = TCP_LAST_ACK;
        } else if (socket->state == TCP_ESTABLISHED) {
            socket->state = TCP_CLOSE_WAIT;
        }
    }
}

void tcp_ack(socket_t *socket, desc_buff_t *buff)
{
    eth_t *eth = (eth_t *)buff->payload;
    ipv4_t *ipv4 = (ipv4_t *)eth->payload;
    tcp_t *tcp = (tcp_t *)ipv4->payload;
    // 接收到的数据包的src_port和dst_port需要交换
    uint16_t dst_port = tcp->src_port;
    tcp->src_port = tcp->dst_port;
    tcp->dst_port = dst_port;

    uint32_t pack_seq_num = ntohl(tcp->seq_num);
    tcp->seq_num = tcp->ack_num;
    socket->seq = ntohl(tcp->seq_num);

    if (tcp->ff.flags == (FLAGS_SYN | FLAGS_ACK) || tcp->ff.flags == (FLAGS_FIN | FLAGS_ACK)) {
        socket->ack = pack_seq_num + 1;
        tcp->ack_num = htonl(socket->ack);
    } else if (tcp->ff.flags == (FLAGS_PSH | FLAGS_ACK)) {
        // 请求头长度
        int hlen = ipv4->ihl * 4 + tcp->ff.offset * 4;
        socket->ack += ntohs(ipv4->total_len) - hlen;
        tcp->ack_num = htonl(socket->ack);
    }
    
    tcp->ff.flags = FLAGS_ACK;
    tcp->window_size = htons(0xFF);
    tcp->checksum = 0;
    ipv4_output(socket->netif, buff, NULL, 0);
    if (socket->socktype == SOCK_CLIENT) {
        if (socket->state == TCP_SYN_SENT) {
            socket->state = TCP_ESTABLISHED;
        } else if (socket->state == TCP_FIN_WAIT_1) {
            socket->state = TCP_FIN_WAIT_2;
        } else if (socket->state == TCP_FIN_WAIT_2) {
            socket->state = TCP_TIME_WAIT;
        }
    } else if (socket->socktype == SOCK_SERVER) {
        if (socket->state == TCP_ESTABLISHED) {
            socket->state = TCP_CLOSE_WAIT;
        } else if (socket->state == TCP_LAST_ACK) {
            socket->state = TCP_CLOSED;
        }
    }
}