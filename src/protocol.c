#include "protocol.h"
#include "config_commands.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    bool is_hotspot;
    uint16_t device_id;      // final assigned id
    uint16_t provisional_id; // temp id used during registration
} ProtocolState;

static ProtocolState state;

static uint16_t generate_id(void){
    return (uint16_t)(rand() & 0xFFFF);
}

void protocol_init(bool is_hotspot){
    state.is_hotspot = is_hotspot;
    state.device_id = is_hotspot ? 0x0000 : 0;
    state.provisional_id = 0;
    srand((unsigned)time(NULL));
}

uint16_t protocol_get_device_id(void){
    return state.device_id;
}

void protocol_request_registration(CharPacket *out){
    if(state.is_hotspot || out == NULL) return;
    state.provisional_id = (uint16_t)(rand() % 1000);
    char msg[32];
    snprintf(msg, sizeof(msg), "{REQ}l{%03u}", state.provisional_id);
    char_packet_push(out, msg);
}

static void handle_req(const char *msg, CharPacket *out){
    if(!state.is_hotspot || out == NULL) return;
    unsigned temp_id = 0;
    if(sscanf(msg, "{REQ}l{%3u}", &temp_id) == 1){
        uint16_t new_id = generate_id();
        char response[48];
        snprintf(response, sizeof(response), "ID:%03u{SET:%04X}k{0000}", temp_id, new_id);
        char_packet_push(out, response);
    }
}

static void handle_set(const char *dest, const char *newid, CharPacket *out){
    if(state.is_hotspot || out == NULL) return;
    if(state.device_id != 0) return; // already set
    unsigned temp = (unsigned)atoi(dest);
    if(temp != state.provisional_id) return; // not for us
    state.device_id = (uint16_t)strtol(newid, NULL, 16);
    char ack[48];
    snprintf(ack, sizeof(ack), "ID:0000{OK}k{%04X}", state.device_id);
    char_packet_push(out, ack);
}

static void handle_ok(const char *src){
    if(state.is_hotspot){
        // For now just print acknowledgment
        printf("Ack from %s\n", src);
    }
}

void protocol_handle_config(const char *msg, CharPacket *out){
    if(msg == NULL) return;
    if(strncmp(msg, "{REQ}l", 6) == 0){
        handle_req(msg, out);
        return;
    }
    char dest[5] = {0};
    char cmd[8] = {0};
    char arg[8] = {0};
    char src[5] = {0};
    // pattern with argument e.g. ID:123{SET:ABCD}k{0000}
    if(sscanf(msg, "ID:%4[^\{]{%3[^}:]:%7[^}]}k{%4[^}]}", dest, cmd, arg, src) == 4){
        ConfigCommand c = config_command_from_string(cmd);
        if(c == CMD_SET){
            handle_set(dest, arg, out);
        }
        return;
    }
    // pattern without argument e.g. ID:0000{OK}k{ABCD}
    if(sscanf(msg, "ID:%4[^\{]{%3[^}]}k{%4[^}]}", dest, cmd, src) == 3){
        ConfigCommand c = config_command_from_string(cmd);
        if(c == CMD_OK){
            handle_ok(src);
        }
        return;
    }
}
