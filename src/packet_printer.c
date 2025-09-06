#include "packet_printer.h"
#include <stdio.h>

void packet_printer_handle(const char *msg) {
    if (msg) {
        printf("Printer: %s\n", msg);
    }
}
