#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdbool.h>
#include <stdint.h>
#include "char_packet.h"

#ifdef __cplusplus
extern "C" {
#endif

void protocol_init(bool is_hotspot);
void protocol_request_registration(CharPacket *out);
void protocol_handle_config(const char *msg, CharPacket *out);
uint16_t protocol_get_device_id(void);

#ifdef __cplusplus
}
#endif

#endif // PROTOCOL_H
