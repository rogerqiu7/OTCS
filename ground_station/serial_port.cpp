#include "serial_port.hpp"

#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace otcs {
namespace {

#ifdef _WIN32

std::string windows_device_path(const std::string& port_name)
{
    if (port_name.rfind(R"(\\.\)", 0) == 0) {
        return port_name;
    }

    return R"(\\.\)" + port_name;
}

std::string last_windows_error_message(const std::string& action)
{
    return action + " failed with Windows error " + std::to_string(GetLastError());
}

#endif

} // namespace

SerialPort::SerialPort(std::string port_name, const std::uint32_t baud_rate)
    : port_name_(std::move(port_name))
{
#ifdef _WIN32
    const std::string device_path = windows_device_path(port_name_);
    handle_ = CreateFileA(device_path.c_str(),
                          GENERIC_READ | GENERIC_WRITE,
                          0,
                          nullptr,
                          OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL,
                          nullptr);

    if (handle_ == INVALID_HANDLE_VALUE) {
        handle_ = nullptr;
        throw std::runtime_error(last_windows_error_message("opening " + port_name_));
    }

    DCB config{};
    config.DCBlength = sizeof(config);
    if (!GetCommState(handle_, &config)) {
        close();
        throw std::runtime_error(last_windows_error_message("reading serial configuration"));
    }

    config.BaudRate = baud_rate;
    config.ByteSize = 8;
    config.Parity = NOPARITY;
    config.StopBits = ONESTOPBIT;

    if (!SetCommState(handle_, &config)) {
        close();
        throw std::runtime_error(last_windows_error_message("applying serial configuration"));
    }

    COMMTIMEOUTS timeouts{};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 100;
    timeouts.ReadTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(handle_, &timeouts)) {
        close();
        throw std::runtime_error(last_windows_error_message("applying serial timeouts"));
    }

    PurgeComm(handle_, PURGE_RXCLEAR | PURGE_TXCLEAR);
#else
    (void)baud_rate;
    throw std::runtime_error("SerialPort is currently implemented for Windows only.");
#endif
}

SerialPort::~SerialPort()
{
    close();
}

SerialPort::SerialPort(SerialPort&& other) noexcept
    : port_name_(std::move(other.port_name_))
    , handle_(other.handle_)
{
#ifdef _WIN32
    other.handle_ = nullptr;
#else
    other.handle_ = -1;
#endif
}

SerialPort& SerialPort::operator=(SerialPort&& other) noexcept
{
    if (this != &other) {
        close();
        port_name_ = std::move(other.port_name_);
        handle_ = other.handle_;
#ifdef _WIN32
        other.handle_ = nullptr;
#else
        other.handle_ = -1;
#endif
    }

    return *this;
}

const std::string& SerialPort::port_name() const
{
    return port_name_;
}

std::string SerialPort::read_line()
{
    while (true) {
        std::string line;
        if (try_read_line(line)) {
            return line;
        }
    }
}

bool SerialPort::try_read_line(std::string& line)
{
    line.clear();

    while (true) {
        char byte = '\0';

#ifdef _WIN32
        DWORD errors = 0;
        COMSTAT status{};
        if (!ClearCommError(handle_, &errors, &status)) {
            throw std::runtime_error(last_windows_error_message("checking serial data"));
        }

        if (status.cbInQue == 0) {
            return false;
        }

        DWORD bytes_read = 0;
        if (!ReadFile(handle_, &byte, 1, &bytes_read, nullptr)) {
            if (GetLastError() == ERROR_OPERATION_ABORTED) {
                ClearCommError(handle_, &errors, &status);
                return false;
            }

            throw std::runtime_error(last_windows_error_message("reading serial data"));
        }

        if (bytes_read == 0) {
            return false;
        }
#else
        throw std::runtime_error("SerialPort is currently implemented for Windows only.");
#endif

        if (byte == '\n') {
            line = std::move(pending_line_);
            pending_line_.clear();
            return true;
        }

        if (byte != '\r') {
            pending_line_.push_back(byte);
        }
    }
}

void SerialPort::write_line(const std::string_view line)
{
    const std::string message = std::string{line} + '\n';
    std::size_t bytes_written_total = 0;

    while (bytes_written_total < message.size()) {
#ifdef _WIN32
        DWORD bytes_written = 0;
        const char* next_byte = message.data() + bytes_written_total;
        const DWORD bytes_remaining = static_cast<DWORD>(message.size() - bytes_written_total);

        if (!WriteFile(handle_, next_byte, bytes_remaining, &bytes_written, nullptr)) {
            throw std::runtime_error(last_windows_error_message("writing serial data"));
        }

        if (bytes_written == 0) {
            throw std::runtime_error("writing serial data failed: no bytes were written");
        }

        bytes_written_total += bytes_written;
#else
        throw std::runtime_error("SerialPort is currently implemented for Windows only.");
#endif
    }
}

void SerialPort::close()
{
#ifdef _WIN32
    if (handle_ != nullptr) {
        CloseHandle(handle_);
        handle_ = nullptr;
    }
#endif
}

} // namespace otcs
