#ifndef CONFIG_COMMANDS_H
#define CONFIG_COMMANDS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CMD_UNKNOWN = 0,
    CMD_REQ,
    CMD_SET,
    CMD_OK
} ConfigCommand;

const char *config_command_to_string(ConfigCommand cmd);
ConfigCommand config_command_from_string(const char *str);

#ifdef __cplusplus
}
#endif

#endif // CONFIG_COMMANDS_H
