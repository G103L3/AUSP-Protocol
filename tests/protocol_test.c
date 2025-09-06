#include <stdio.h>
#include "../src/protocol.h"
#include "../src/char_packet.h"

int main(void){
    CharPacket out;
    char_packet_init(&out);
    protocol_init(true); // hotspot
    protocol_handle_config("{REQ}l{123}", &out);
    char buf[64];
    if(char_packet_pop(&out, buf, sizeof(buf))){
        printf("%s\n", buf);
    }
    return 0;
}
