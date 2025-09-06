#include "packet_router.h"
#include "packet_printer.h"
#include <string.h>

static CharPacketQueue master_out;
static CharPacketQueue slave_out;
static CharPacketQueue config_out;

void packet_router_init(void) {
    char_packet_init(&master_out);
    char_packet_init(&slave_out);
    char_packet_init(&config_out);
}

static CharPacketQueue* queue_for_channel(PacketChannel ch) {
    switch (ch) {
        case CHANNEL_MASTER: return &master_out;
        case CHANNEL_SLAVE:  return &slave_out;
        default:             return &config_out;
    }
}

static bool contains_five(const char *msg) {
    return (msg && strchr(msg, '5') != NULL);
}

void packet_router_process_ascii(PacketChannel ch, const char *ascii) {
    if (!ascii) return;
    const char *start = ascii;
    while (*start) {
        const char *end = strchr(start, CHAR_PACKET_SEPARATOR);
        size_t len = end ? (size_t)(end - start) : strlen(start);
        if (len > 0) {
            char buf[CHAR_PACKET_MAX_LEN];
            if (len >= CHAR_PACKET_MAX_LEN) len = CHAR_PACKET_MAX_LEN - 1;
            memcpy(buf, start, len);
            buf[len] = '\0';
            if (contains_five(buf)) {
                packet_printer_handle(buf);
            } else {
                char_packet_push(queue_for_channel(ch), buf);
            }
        }
        if (!end) break;
        start = end + 1;
    }
}

bool packet_router_pop(PacketChannel ch, char *out) {
    return char_packet_pop(queue_for_channel(ch), out);
}
