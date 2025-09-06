#include "char_packet.h"
#include <string.h>

void char_packet_init(CharPacketQueue *q) {
    q->head = 0;
    q->tail = 0;
    q->count = 0;
    for (int i = 0; i < CHAR_PACKET_MAX_PACKETS; ++i) {
        q->data[i][0] = '\0';
    }
}

bool char_packet_push(CharPacketQueue *q, const char *msg) {
    if (q->count >= CHAR_PACKET_MAX_PACKETS || !msg) {
        return false;
    }
    size_t len = strnlen(msg, CHAR_PACKET_MAX_LEN - 1);
    memcpy(q->data[q->tail], msg, len);
    q->data[q->tail][len] = '\0';
    q->tail = (q->tail + 1) % CHAR_PACKET_MAX_PACKETS;
    q->count++;
    return true;
}

bool char_packet_pop(CharPacketQueue *q, char *out) {
    if (q->count == 0 || !out) {
        return false;
    }
    strncpy(out, q->data[q->head], CHAR_PACKET_MAX_LEN);
    out[CHAR_PACKET_MAX_LEN-1] = '\0';
    q->data[q->head][0] = '\0';
    q->head = (q->head + 1) % CHAR_PACKET_MAX_PACKETS;
    q->count--;
    return true;
}

int char_packet_count(const CharPacketQueue *q) {
    return q->count;
}
