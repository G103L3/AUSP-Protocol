#include "protocol.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "config_commands.h"

#define MAX_DEVICES 8

typedef struct {
    uint16_t temp_id;   // three digit temporary id
    uint16_t final_id;  // four digit hex id assigned by hotspot
    bool in_use;
} DeviceEntry;

static DeviceEntry devices[MAX_DEVICES];
static uint16_t next_final_id = 1; // start from 0x0001

static DeviceEntry *find_by_temp(uint16_t temp){
    for(size_t i=0;i<MAX_DEVICES;i++)
        if(devices[i].in_use && devices[i].temp_id==temp)
            return &devices[i];
    return NULL;
}

static DeviceEntry *allocate_entry(uint16_t temp){
    for(size_t i=0;i<MAX_DEVICES;i++){
        if(!devices[i].in_use){
            devices[i].in_use=true;
            devices[i].temp_id=temp;
            devices[i].final_id=next_final_id++;
            return &devices[i];
        }
    }
    return NULL;
}

void protocol_init(ProtocolState *ps, bool hotspot){
    memset(ps,0,sizeof(*ps));
    ps->is_hotspot = hotspot;
    ps->self_id = hotspot ? 0x0000 : PROTOCOL_ID_UNASSIGNED;
    ps->temp_id = PROTOCOL_TEMP_ID_NONE;
    ps->next_seq = 1;
    memset(devices,0,sizeof(devices));
    next_final_id = 1;
}

size_t protocol_build_config_request(ProtocolState *ps, char *out, size_t out_size){
    if(ps->is_hotspot || ps->temp_id != PROTOCOL_TEMP_ID_NONE)
        return 0; // nothing to build
    ps->temp_id = (uint16_t)(rand() % 900 + 100); // three digit 100..999
    snprintf(out, out_size, "{REQ}l{%03u}", ps->temp_id);
    return strlen(out);
}

static bool parse_id(const char *p, uint16_t *id){
    if(!p) return false;
    *id = (uint16_t)strtoul(p, NULL, 16);
    return true;
}

bool protocol_handle_packet(ProtocolState *ps, const char *in, char *out, size_t out_size){
    if(out_size>0) out[0]='\0';
    if(ps->is_hotspot){
        // hotspot behaviour
        if(strncmp(in, "{REQ}l{",7)==0){
            // new device request
            uint16_t temp = (uint16_t)atoi(in+7);
            DeviceEntry *e = allocate_entry(temp);
            if(e){
                snprintf(out,out_size,"ID:%03u{SET:%04X}k{0000}", temp, e->final_id);
                return true;
            }
        } else if(strncmp(in,"ID:0000",8)==0){
            // message addressed to hotspot
            const char *cmd_start = strchr(in,'{');
            if(cmd_start){
                char cmd[8];
                const char *end = strchr(cmd_start,'}');
                size_t len = end ? (size_t)(end-cmd_start-1) : 0;
                if(len < sizeof(cmd)){
                    strncpy(cmd, cmd_start+1, len);
                    cmd[len] = '\0';
                    ConfigCommand c = config_command_from_string(cmd);
                    if(c == CMD_OK){
                        const char *src = strstr(in,"k{");
                        if(src){
                            uint16_t sid; parse_id(src+2,&sid);
                            for(size_t i=0;i<MAX_DEVICES;i++){
                                if(devices[i].in_use && devices[i].final_id==sid){
                                    // device confirmed id assignment
                                    return false;
                                }
                            }
                        }
                    }
                }
            }
        }
    } else {
        // client behaviour
        if(ps->self_id==PROTOCOL_ID_UNASSIGNED){
            // waiting for SET
            if(strncmp(in,"ID:",3)==0){
                uint16_t dst; parse_id(in+3,&dst);
                if(dst==ps->temp_id){
                    const char *cmd_start = strchr(in,'{');
                    if(cmd_start){
                        const char *end = strchr(cmd_start,'}');
                        if(end){
                            char cmd[16];
                            size_t len = end - cmd_start - 1;
                            if(len < sizeof(cmd)){
                                strncpy(cmd, cmd_start+1, len);
                                cmd[len]='\0';
                                if(config_command_from_string(cmd) == CMD_SET){
                                    uint16_t new_id; parse_id(cmd_start+5,&new_id);
                                    ps->self_id = new_id;
                                    snprintf(out,out_size,"ID:0000{OK}k{%04X}", ps->self_id);
                                    return true;
                                }
                            }
                        }
                    }
                }
            }
        } else {
            // further messages could be handled here
            (void)in;
        }
    }
    return false;
}
