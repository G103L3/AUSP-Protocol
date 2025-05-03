#ifndef SERIAL_BRIDGE_H
#define SERIAL_BRIDGE_H

#ifdef __cplusplus
extern "C" {
#endif

void serial_init(unsigned long baudrate);
void serial_write_string(const char* str);
void serial_write_formatted(const char* format, ...);

#ifdef __cplusplus
}
#endif

#endif