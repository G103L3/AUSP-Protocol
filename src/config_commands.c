#include "config_commands.h"
#include <string.h>

const char *config_command_to_string(ConfigCommand cmd){
    switch(cmd){
        case CMD_REQ: return "REQ";
        case CMD_SET: return "SET";
        case CMD_OK:  return "OK";
        default:      return "";
    }
}

ConfigCommand config_command_from_string(const char *str){
    if(str == NULL) return CMD_UNKNOWN;
    if(strcmp(str, "REQ") == 0) return CMD_REQ;
    if(strcmp(str, "SET") == 0) return CMD_SET;
    if(strcmp(str, "OK")  == 0) return CMD_OK;
    return CMD_UNKNOWN;
}
