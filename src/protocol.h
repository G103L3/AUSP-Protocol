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
void protocol_send_command(const char *dest_id, const char *operation);
void protocol_list_devices(char *buf, size_t buflen);

typedef void (*ProtocolMessageCallback)(const char *msg);
void protocol_set_message_callback(ProtocolMessageCallback cb);

#ifdef __cplusplus
}
#endif

#endif // PROTOCOL_H
