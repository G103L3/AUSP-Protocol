#ifndef CONFIG_COMMANDS_H
#define CONFIG_COMMANDS_H

#include <stdbool.h>
#include "protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

void config_commands_init(bool is_hotspot);
void config_commands_handle(const ProtocolPacket *pkt);
const char *config_commands_get_id(void);

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_COMMANDS_H */
