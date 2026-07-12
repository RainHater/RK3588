#ifndef LOGGER_H
#define LOGGER_H

#include <spdlog/logger.h>

#include <memory>
#include <string>

class LoggerWithTag : public spdlog::logger
{
public:
    LoggerWithTag(
        const std::string& tag,
        spdlog::sinks_init_list sinks
    );

    ~LoggerWithTag() override = default;

    /**
     * 获取或创建指定名称的 Logger。
     *
     * 相同 tag 会返回同一个 Logger，避免重复注册。
     */
    static std::shared_ptr<LoggerWithTag> GetLogger(
        const std::string& tag
    );

private:
    /**
     * 根据当前日期和小时生成日志文件路径。
     *
     * 示例：
     * logs/2026-07-12/07-12-13.log
     */
    static std::string getTimeStampedLogFile();
};

#endif // LOGGER_H