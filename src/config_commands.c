#include "config_commands.h"
#include <string.h>

ConfigCommand config_command_from_string(const char *s){
    if(!s) return CMD_UNKNOWN;
    if(strncmp(s,"REQ",3)==0) return CMD_REQ;
    if(strncmp(s,"SET",3)==0) return CMD_SET;
    if(strncmp(s,"OK",2)==0) return CMD_OK;
    if(strncmp(s,"ACK",3)==0) return CMD_ACK;
    if(strncmp(s,"TOKEN",5)==0) return CMD_TOKEN;
    if(strncmp(s,"EXT",3)==0) return CMD_EXT;
    return CMD_UNKNOWN;
}
