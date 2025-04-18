#ifndef CSOS_INET_H
#define CSOS_INET_H

#include <netx/arp_map.h>

void arpl(arp_map_data_t *arp_data);
void arpc();
void ping(const char *ip);

#endif