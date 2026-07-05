#include <cstdio>
#include <string>

#include "flight_computer.hpp"
#include "pico/stdlib.h"
#include "text_protocol.hpp"

int main()
{
    stdio_init_all();

    // USB serial appears a moment after firmware starts. Waiting here keeps the
    // first boot telemetry from being printed before the PC has opened the COM
    // port.
    sleep_ms(2000);

    otcs::FlightComputer flight_computer;

    while (true) {
        // One loop is one spacecraft telemetry cycle. The shared flight logic
        // advances by one second, then the shared protocol formats the snapshot
        // into the same text line the future Ground Station will parse.
        const otcs::TelemetrySnapshot telemetry = flight_computer.tick(1000);
        const std::string message = otcs::format_telemetry(telemetry);

        std::printf("%s\n", message.c_str());
        sleep_ms(1000);
    }
}
