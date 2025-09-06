#ifndef CONFIG_COMMANDS_H
#define CONFIG_COMMANDS_H

typedef enum {
    CMD_UNKNOWN,
    CMD_REQ,
    CMD_SET,
    CMD_OK,
    CMD_ACK,
    CMD_TOKEN,
    CMD_EXT
} ConfigCommand;

ConfigCommand config_command_from_string(const char *s);

#endif // CONFIG_COMMANDS_H
