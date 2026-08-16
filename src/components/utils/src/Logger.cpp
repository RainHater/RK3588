#include "Logger.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <ctime>
#include <filesystem>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <stdexcept>

LoggerWithTag::LoggerWithTag(
    const std::string& tag,
    spdlog::sinks_init_list sinks
)
    : spdlog::logger(tag, sinks)
{
    // 格式：时间 [Logger名称] [日志级别] 消息
    set_pattern("[%T] [%n] [%^%l%$] %v");

    set_level(spdlog::level::debug);

    // warning 及以上日志立即写入文件
    flush_on(spdlog::level::warn);
}

std::shared_ptr<LoggerWithTag> LoggerWithTag::GetLogger(
    const std::string& tag
)
{
    /*
     * 防止多个线程同时创建相同名称的 Logger。
     */
    static std::mutex logger_mutex;
    std::lock_guard<std::mutex> lock(logger_mutex);

    /*
     * 如果已经存在相同名称的 Logger，直接返回。
     */
    const auto existing_logger = spdlog::get(tag);

    if (existing_logger)
    {
        const auto logger =
            std::dynamic_pointer_cast<LoggerWithTag>(
                existing_logger
            );

        if (logger)
        {
            return logger;
        }

        throw std::runtime_error(
            "Logger name already exists but type does not match: "
            + tag
        );
    }

    /*
     * 所有 LoggerWithTag 共用同一个控制台 Sink。
     */
    static const auto console_sink =
        std::make_shared<
            spdlog::sinks::stdout_color_sink_mt
        >();

    /*
     * 所有 LoggerWithTag 共用同一个日志文件 Sink。
     *
     * 第二个参数 true 表示以追加方式打开文件，
     * 不会清空已有日志。
     */
    static const auto file_sink =
        std::make_shared<
            spdlog::sinks::basic_file_sink_mt
        >(
            getTimeStampedLogFile(),
            true
        );

    auto logger = std::make_shared<LoggerWithTag>(
        tag,
        spdlog::sinks_init_list{
            console_sink,
            file_sink
        }
    );

    spdlog::register_logger(logger);

    return logger;
}

std::string LoggerWithTag::getTimeStampedLogFile()
{
    const std::time_t current_time = std::time(nullptr);

    std::tm local_time{};

#ifdef _WIN32
    localtime_s(&local_time, &current_time);
#else
    localtime_r(&current_time, &local_time);
#endif

    /*
     * 日志目录：
     * logs/2026-07-12
     */
    std::ostringstream folder_stream;

    folder_stream
        << "logs/"
        << std::put_time(
            &local_time,
            "%Y-%m-%d"
        );

    const std::filesystem::path folder_path =
        folder_stream.str();

    std::error_code error_code;

    std::filesystem::create_directories(
        folder_path,
        error_code
    );

    if (error_code)
    {
        throw std::runtime_error(
            "Failed to create log directory: "
            + folder_path.string()
            + ", error: "
            + error_code.message()
        );
    }

    /*
     * 日志文件：
     * 07-12-13.log
     */
    std::ostringstream filename_stream;

    filename_stream
        << std::put_time(
            &local_time,
            "%m-%d-%H"
        )
        << ".log";

    return (
        folder_path / filename_stream.str()
    ).string();
}