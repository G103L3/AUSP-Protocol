#ifndef PACKET_ROUTER_H
#define PACKET_ROUTER_H

#include "char_packet.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CHANNEL_MASTER,
    CHANNEL_SLAVE,
    CHANNEL_CONFIG
} PacketChannel;

void packet_router_init(void);
void packet_router_process_ascii(PacketChannel ch, const char *ascii);
bool packet_router_pop(PacketChannel ch, char *out);

#ifdef __cplusplus
}
#endif

#endif
