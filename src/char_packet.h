#ifndef CHAR_PACKET_H
#define CHAR_PACKET_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CHAR_PACKET_MAX_PACKETS 8
#define CHAR_PACKET_MAX_LEN     64
#define CHAR_PACKET_SEPARATOR   '|'

typedef struct {
    char data[CHAR_PACKET_MAX_PACKETS][CHAR_PACKET_MAX_LEN];
    int head;
    int tail;
    int count;
} CharPacketQueue;

void char_packet_init(CharPacketQueue *q);
bool char_packet_push(CharPacketQueue *q, const char *msg);
bool char_packet_pop(CharPacketQueue *q, char *out);
int  char_packet_count(const CharPacketQueue *q);

#ifdef __cplusplus
}
#endif

#endif
