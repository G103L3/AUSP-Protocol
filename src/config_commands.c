#include "config_commands.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

static bool g_is_hotspot = false;
static char g_device_id[PROTOCOL_ID_SIZE] = "FFFF"; /* unknown */
static unsigned g_next_id = 1; /* hotspot id counter */

static void generate_hex_id(char out[PROTOCOL_ID_SIZE]){
    snprintf(out, PROTOCOL_ID_SIZE, "%04X", g_next_id++ & 0xFFFF);
}

void config_commands_init(bool is_hotspot){
    g_is_hotspot = is_hotspot;
    if(g_is_hotspot){
        strcpy(g_device_id, "0000");
    }else{
        /* seed and create a provisional decimal id */
        srand((unsigned)time(NULL));
        int temp = rand() % 1000; /* 3 digits */
        snprintf(g_device_id, PROTOCOL_ID_SIZE, "%03d", temp);
        ProtocolPacket req = {0};
        req.op = PROTOCOL_OP_REQ;
        req.kind = PROTOCOL_KIND_LINK; /* provisional */
        strcpy(req.sender_id, g_device_id);
        char buf[64];
        protocol_format(&req, buf, sizeof(buf));
        printf("CONFIG TX: %s\n", buf);
    }
}

const char *config_commands_get_id(void){
    return g_device_id;
}

void config_commands_handle(const ProtocolPacket *pkt){
    if(g_is_hotspot){
        if(pkt->op == PROTOCOL_OP_REQ && pkt->kind == PROTOCOL_KIND_LINK){
            char new_id[PROTOCOL_ID_SIZE];
            generate_hex_id(new_id);
            ProtocolPacket resp = {0};
            strcpy(resp.dest_id, pkt->sender_id); /* provisional id */
            resp.op = PROTOCOL_OP_SET;
            resp.kind = PROTOCOL_KIND_MASTER;
            strcpy(resp.data, new_id);
            strcpy(resp.sender_id, g_device_id); /* hotspot id */
            char buf[64];
            protocol_format(&resp, buf, sizeof(buf));
            printf("CONFIG TX: %s\n", buf);
        }else if(pkt->op == PROTOCOL_OP_OK){
            printf("Device %s registered\n", pkt->sender_id);
        }
    }else{
        if(pkt->op == PROTOCOL_OP_SET){
            /* hotspot assigns our permanent id */
            strcpy(g_device_id, pkt->data);
            ProtocolPacket ack = {0};
            strcpy(ack.dest_id, pkt->sender_id);
            ack.op = PROTOCOL_OP_OK;
            ack.kind = PROTOCOL_KIND_MASTER;
            strcpy(ack.sender_id, g_device_id);
            char buf[64];
            protocol_format(&ack, buf, sizeof(buf));
            printf("CONFIG TX: %s\n", buf);
        }
    }
}
