#include "Logger.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include <filesystem>

bool Logger::s_inited = false;

void Logger::init(const std::string& logDir)
{
    if (s_inited)
        return;

    std::filesystem::create_directories(logDir);
    createDefaultLogger(logDir);

    s_inited = true;
}

void Logger::createDefaultLogger(const std::string& logDir)
{
    auto console_sink =
        std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

    auto file_sink =
        std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            logDir + "/app.log",
            5 * 1024 * 1024,   // 5MB
            3                 // 3 files
        );

    auto logger = std::make_shared<spdlog::logger>(
        "APP",
        spdlog::sinks_init_list{console_sink, file_sink}
    );

#ifdef NDEBUG
    logger->set_level(spdlog::level::info);
#else
    logger->set_level(spdlog::level::trace);
#endif

    logger->set_pattern(
        "[%Y-%m-%d %H:%M:%S.%e] "
        "[%^%l%$] "
        "[%n] "
        "%v"
    );

    spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);
}

std::shared_ptr<spdlog::logger> Logger::defaultLogger()
{
    return spdlog::default_logger();
}

std::shared_ptr<spdlog::logger> Logger::get(const std::string& name)
{
    auto logger = spdlog::get(name);
    if (logger)
        return logger;

    auto base = spdlog::default_logger();
    auto new_logger = base->clone(name);

    spdlog::register_logger(new_logger);
    return new_logger;
}

bool Logger::isInited()
{
    return s_inited;
}
