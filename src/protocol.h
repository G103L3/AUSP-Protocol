#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PROTOCOL_ID_SIZE 5
#define PROTOCOL_DATA_SIZE 16
#define PROTOCOL_OP_SIZE 16

/* Supported operations for the configuration channel.  */
typedef enum {
    PROTOCOL_OP_REQ,   /* device requests an id          */
    PROTOCOL_OP_SET,   /* hotspot assigns new id         */
    PROTOCOL_OP_OK,    /* device acknowledges assignment */
    PROTOCOL_OP_UNKNOWN
} ProtocolOp;

/* Packet type as indicated by the character between the two blocks. */
typedef enum {
    PROTOCOL_KIND_MASTER, /* 'k' */
    PROTOCOL_KIND_SLAVE,  /* 'z' */
    PROTOCOL_KIND_LINK    /* 'l' initial handshake */
} ProtocolKind;

/* High level parsed packet representation. */
typedef struct {
    char dest_id[PROTOCOL_ID_SIZE];
    char sender_id[PROTOCOL_ID_SIZE];
    ProtocolOp op;
    char data[PROTOCOL_DATA_SIZE];
    ProtocolKind kind;
    unsigned sequence; /* optional sequence number */
    bool ack;          /* ack flag */
} ProtocolPacket;

bool protocol_parse(const char *msg, ProtocolPacket *out);
size_t protocol_format(const ProtocolPacket *pkt, char *buffer, size_t buf_size);

#ifdef __cplusplus
}
#endif

#endif /* PROTOCOL_H */
