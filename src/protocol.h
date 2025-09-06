#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define PROTOCOL_MAX_PACKET 64
#define PROTOCOL_ID_UNASSIGNED 0xFFFF
#define PROTOCOL_TEMP_ID_NONE 0

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t self_id;       // 0x0000 for hotspot, assigned id otherwise
    uint16_t temp_id;       // temporary numeric id before assignment
    bool is_hotspot;        // true for hotspot device
    uint8_t next_seq;       // sequence number for outgoing packets
} ProtocolState;

void protocol_init(ProtocolState *ps, bool hotspot);

// Build initial configuration request for non-hotspot device
// Returns length of packet written to out (zero if not available)
size_t protocol_build_config_request(ProtocolState *ps, char *out, size_t out_size);

// Handle incoming packet. If a response should be sent, it is written to out and
// function returns true. Otherwise returns false.
bool protocol_handle_packet(ProtocolState *ps, const char *in, char *out, size_t out_size);

#ifdef __cplusplus
}
#endif

#endif // PROTOCOL_H
