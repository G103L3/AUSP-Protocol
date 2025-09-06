#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdbool.h>
#include "char_packet_router.h"

#ifdef __cplusplus
extern "C" {
#endif

void protocol_init(bool is_hotspot);
void protocol_handle_message(ChannelType ch, const char *msg);
void protocol_tick(void);
const char* protocol_device_id(void);
void protocol_grant_token(const char *dest_id, unsigned int seconds);
bool protocol_can_send(void);

#ifdef __cplusplus
}
#endif

#endif // PROTOCOL_H
