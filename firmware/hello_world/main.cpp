#include <cstdio>

#include "pico/stdlib.h"

int main()
{
    stdio_init_all();

    // Give the host a moment to enumerate the USB serial device before the
    // first message is printed.
    sleep_ms(2000);

    unsigned int message_count = 0;

    while (true) {
        ++message_count;
        std::printf("OTCS custom Pico hello world\n");
        std::printf("Board: Raspberry Pi Pico 2 W\n");
        std::printf("Message count: %u\n\n", message_count);
        sleep_ms(1000);
    }
}
