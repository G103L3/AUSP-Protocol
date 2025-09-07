#include "command_dict.h"
#include <string.h>

typedef struct {
    const char *name;
    Command cmd;
} CmdEntry;

static const CmdEntry dict[] = {
    {"REQ", CMD_REQ},
    {"SET", CMD_SET},
    {"OK",  CMD_OK},
    {"TKN", CMD_TKN},
    {"MOVEMENT", CMD_MOVEMENT},
    {"EXT", CMD_EXT},
    {NULL,   CMD_UNKNOWN}
};

Command command_from_string(const char *s){
    for(const CmdEntry *e = dict; e->name; ++e){
        if(strcmp(e->name, s) == 0)
            return e->cmd;
    }
    return CMD_UNKNOWN;
}

const char* command_to_string(Command cmd){
    for(const CmdEntry *e = dict; e->name; ++e){
        if(e->cmd == cmd)
            return e->name;
    }
    return "UNKNOWN";
}
