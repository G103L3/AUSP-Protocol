#include "protocol.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

static ProtocolOp parse_operation(const char *op, char *data_out, size_t data_size){
    if(strncmp(op, "REQ", 3) == 0){
        data_out[0] = '\0';
        return PROTOCOL_OP_REQ;
    }
    if(strncmp(op, "OK", 2) == 0){
        data_out[0] = '\0';
        return PROTOCOL_OP_OK;
    }
    if(strncmp(op, "SET:", 4) == 0){
        strncpy(data_out, op+4, data_size-1);
        data_out[data_size-1] = '\0';
        return PROTOCOL_OP_SET;
    }
    /* Unknown operation, copy raw data. */
    strncpy(data_out, op, data_size-1);
    data_out[data_size-1] = '\0';
    return PROTOCOL_OP_UNKNOWN;
}

bool protocol_parse(const char *msg, ProtocolPacket *out){
    memset(out, 0, sizeof(*out));
    const char *p = msg;
    if(strncmp(p, "ID:", 3) == 0){
        p += 3;
        strncpy(out->dest_id, p, PROTOCOL_ID_SIZE-1);
        out->dest_id[PROTOCOL_ID_SIZE-1] = '\0';
        p += strlen(out->dest_id);
    }
    if(*p != '{')
        return false;
    const char *close = strchr(p, '}');
    if(!close) return false;
    char opbuf[PROTOCOL_OP_SIZE];
    size_t len = (size_t)(close - p - 1);
    if(len >= sizeof(opbuf)) len = sizeof(opbuf)-1;
    strncpy(opbuf, p+1, len);
    opbuf[len] = '\0';
    out->op = parse_operation(opbuf, out->data, sizeof(out->data));
    p = close + 1;
    if(*p == 'k') out->kind = PROTOCOL_KIND_MASTER;
    else if(*p == 'z') out->kind = PROTOCOL_KIND_SLAVE;
    else out->kind = PROTOCOL_KIND_LINK;
    p++;
    if(*p != '{') return false;
    close = strchr(p, '}');
    if(!close) return false;
    len = (size_t)(close - p - 1);
    if(len >= PROTOCOL_ID_SIZE) len = PROTOCOL_ID_SIZE-1;
    strncpy(out->sender_id, p+1, len);
    out->sender_id[len] = '\0';
    return true;
}

size_t protocol_format(const ProtocolPacket *pkt, char *buffer, size_t buf_size){
    char opbuf[PROTOCOL_OP_SIZE];
    switch(pkt->op){
        case PROTOCOL_OP_REQ:
            strcpy(opbuf, "REQ");
            break;
        case PROTOCOL_OP_OK:
            strcpy(opbuf, "OK");
            break;
        case PROTOCOL_OP_SET:
            snprintf(opbuf, sizeof(opbuf), "SET:%s", pkt->data);
            break;
        default:
            strncpy(opbuf, pkt->data, sizeof(opbuf)-1);
            opbuf[sizeof(opbuf)-1] = '\0';
            break;
    }
    char kind_char = 'k';
    if(pkt->kind == PROTOCOL_KIND_SLAVE) kind_char = 'z';
    else if(pkt->kind == PROTOCOL_KIND_LINK) kind_char = 'l';
    if(pkt->dest_id[0]){
        return snprintf(buffer, buf_size, "ID:%s{%s}%c{%s}", pkt->dest_id, opbuf, kind_char, pkt->sender_id);
    }else{
        return snprintf(buffer, buf_size, "{%s}%c{%s}", opbuf, kind_char, pkt->sender_id);
    }
}
