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
void protocol_send_command(const char *dest_id, const char *operation);
void protocol_send_movement_request(const char *dest_id, unsigned long duration_ms);
void protocol_send_response(const char *operation);
void protocol_send_abort(void);
void protocol_list_devices(char *buf, size_t buflen);

typedef void (*ProtocolMessageCallback)(const char *msg);
void protocol_set_message_callback(ProtocolMessageCallback cb);

#ifdef __cplusplus
}
#endif

#endif // PROTOCOL_H
