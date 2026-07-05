#include "mission_logger.hpp"

#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace otcs {
namespace {

std::string padded_index(const int index)
{
    std::ostringstream output;
    output << std::setw(3) << std::setfill('0') << index;
    return output.str();
}

} // namespace

MissionLogger::MissionLogger(const std::filesystem::path& log_directory)
{
    std::filesystem::create_directories(log_directory);

    events_path_ = next_log_path(log_directory, "events_", ".log");
    telemetry_path_ = next_log_path(log_directory, "telemetry_", ".csv");

    events_log_.open(events_path_, std::ios::out);
    if (!events_log_) {
        throw std::runtime_error("failed to open event log: " + events_path_.string());
    }

    telemetry_log_.open(telemetry_path_, std::ios::out);
    if (!telemetry_log_) {
        throw std::runtime_error("failed to open telemetry log: " + telemetry_path_.string());
    }

    telemetry_log_ << "host_time,timestamp_ms,sequence,spacecraft_id,mode,battery_percent,temperature_c,"
                      "fault_flags,uptime_ms\n";
}

const std::filesystem::path& MissionLogger::events_path() const
{
    return events_path_;
}

const std::filesystem::path& MissionLogger::telemetry_path() const
{
    return telemetry_path_;
}

void MissionLogger::log_connected(const std::string_view port_name, const std::uint32_t baud_rate)
{
    events_log_ << '[' << timestamp() << "] CONNECTED " << port_name << ' ' << baud_rate << '\n';
    events_log_.flush();
}

void MissionLogger::log_rx(const std::string_view line)
{
    events_log_ << '[' << timestamp() << "] RX " << line << '\n';
    events_log_.flush();
}

void MissionLogger::log_tx(const std::string_view line)
{
    events_log_ << '[' << timestamp() << "] TX " << line << '\n';
    events_log_.flush();
}

void MissionLogger::log_note(const std::string_view message)
{
    events_log_ << '[' << timestamp() << "] " << message << '\n';
    events_log_.flush();
}

void MissionLogger::log_telemetry(const TelemetrySnapshot& telemetry)
{
    telemetry_log_ << timestamp() << ',' << telemetry.timestamp_ms << ',' << telemetry.sequence << ','
                   << static_cast<unsigned int>(telemetry.spacecraft_id) << ',' << to_string(telemetry.mode)
                   << ',' << static_cast<unsigned int>(telemetry.battery_percent) << ','
                   << telemetry.temperature_c << ',' << telemetry.fault_flags << ',' << telemetry.uptime_ms
                   << '\n';
    telemetry_log_.flush();
}

std::filesystem::path MissionLogger::next_log_path(const std::filesystem::path& directory,
                                                   const std::string_view prefix,
                                                   const std::string_view extension)
{
    for (int index = 1; index < 1000; ++index) {
        const std::filesystem::path candidate =
            directory / (std::string{prefix} + padded_index(index) + std::string{extension});
        if (!std::filesystem::exists(candidate)) {
            return candidate;
        }
    }

    throw std::runtime_error("no available log file names remain in " + directory.string());
}

std::string MissionLogger::timestamp()
{
    const std::time_t now = std::time(nullptr);
    std::tm local_time{};

#ifdef _WIN32
    localtime_s(&local_time, &now);
#else
    localtime_r(&now, &local_time);
#endif

    std::ostringstream output;
    output << std::put_time(&local_time, "%H:%M:%S");
    return output.str();
}

} // namespace otcs
