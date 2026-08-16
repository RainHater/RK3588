#include "Logger.h"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <ctime>
#include <filesystem>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace rkplatform::component::logging {
namespace {

std::mutex& GetLoggerMutex() {
    static std::mutex logger_mutex;
    return logger_mutex;
}

std::unordered_map<std::string, std::weak_ptr<spdlog::logger>>&
GetOwnedLoggers() {
    static std::unordered_map<std::string, std::weak_ptr<spdlog::logger>>
        owned_loggers;
    return owned_loggers;
}

std::string& GetConfiguredLogFile() {
    static std::string configured_log_file;
    return configured_log_file;
}

std::shared_ptr<spdlog::sinks::stdout_color_sink_mt>& GetConsoleSink() {
    static std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> console_sink;
    return console_sink;
}

std::shared_ptr<spdlog::sinks::basic_file_sink_mt>& GetFileSink() {
    static std::shared_ptr<spdlog::sinks::basic_file_sink_mt> file_sink;
    return file_sink;
}

void EnsureLogDirectory(const std::string& log_file) {
    const auto folder_path = std::filesystem::path(log_file).parent_path();
    if (folder_path.empty()) {
        return;
    }

    std::error_code error_code;
    std::filesystem::create_directories(folder_path, error_code);
    if (error_code) {
        throw std::runtime_error(
            "Failed to create log directory: " + folder_path.string() +
            ", error: " + error_code.message()
        );
    }
}

std::string GetTimeStampedLogFile() {
    const std::time_t current_time = std::time(nullptr);
    std::tm local_time{};

#ifdef _WIN32
    localtime_s(&local_time, &current_time);
#else
    localtime_r(&current_time, &local_time);
#endif

    std::ostringstream folder_stream;
    folder_stream << "logs/" << std::put_time(&local_time, "%Y-%m-%d");
    const std::filesystem::path folder_path = folder_stream.str();

    std::ostringstream filename_stream;
    filename_stream << std::put_time(&local_time, "%m-%d-%H") << ".log";
    return (folder_path / filename_stream.str()).string();
}

}  // namespace

void SetLogPath(const std::string& log_file_path) {
    if (log_file_path.empty()) {
        throw std::invalid_argument("Log file path cannot be empty");
    }

    std::lock_guard<std::mutex> lock(GetLoggerMutex());
    if (GetFileSink()) {
        throw std::logic_error(
            "Log file path must be set before the first logger is created"
        );
    }
    GetConfiguredLogFile() = log_file_path;
}

std::shared_ptr<spdlog::logger> GetLogger(const std::string& tag) {
    std::lock_guard<std::mutex> lock(GetLoggerMutex());
    auto& owned_loggers = GetOwnedLoggers();

    const auto registered_logger = spdlog::get(tag);
    const auto owned_iterator = owned_loggers.find(tag);
    if (owned_iterator != owned_loggers.end()) {
        if (const auto owned_logger = owned_iterator->second.lock()) {
            if (registered_logger && registered_logger != owned_logger) {
                throw std::runtime_error(
                    "Logger name is registered by another owner: " + tag
                );
            }
            if (!registered_logger) {
                spdlog::register_logger(owned_logger);
            }
            return owned_logger;
        }
        owned_loggers.erase(owned_iterator);
    }

    if (registered_logger) {
        throw std::runtime_error(
            "Logger name is registered by another owner: " + tag
        );
    }

    auto& console_sink = GetConsoleSink();
    auto& file_sink = GetFileSink();
    if (!file_sink) {
        console_sink =
            std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        const auto& configured_log_file = GetConfiguredLogFile();
        const std::string log_file = configured_log_file.empty()
            ? GetTimeStampedLogFile()
            : configured_log_file;
        EnsureLogDirectory(log_file);
        file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
            log_file,
            true
        );
    }

    auto logger = std::make_shared<spdlog::logger>(
        tag,
        spdlog::sinks_init_list{console_sink, file_sink}
    );
    logger->set_pattern("[%T] [%n] [%^%l%$] %v");
    logger->set_level(spdlog::level::debug);
    logger->flush_on(spdlog::level::warn);
    spdlog::register_logger(logger);
    owned_loggers.emplace(tag, logger);
    return logger;
}

}  // namespace rkplatform::component::logging
