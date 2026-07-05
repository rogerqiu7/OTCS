#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>

#include "spacecraft_types.hpp"

namespace otcs {

class MissionLogger {
public:
    explicit MissionLogger(const std::filesystem::path& log_directory = "logs");

    [[nodiscard]] const std::filesystem::path& events_path() const;
    [[nodiscard]] const std::filesystem::path& telemetry_path() const;

    void log_connected(std::string_view port_name, std::uint32_t baud_rate);
    void log_rx(std::string_view line);
    void log_tx(std::string_view line);
    void log_note(std::string_view message);
    void log_telemetry(const TelemetrySnapshot& telemetry);

private:
    static std::filesystem::path next_log_path(const std::filesystem::path& directory,
                                               std::string_view prefix,
                                               std::string_view extension);
    static std::string timestamp();

    std::filesystem::path events_path_;
    std::filesystem::path telemetry_path_;
    std::ofstream events_log_;
    std::ofstream telemetry_log_;
};

} // namespace otcs
