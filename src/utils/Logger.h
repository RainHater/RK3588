#ifndef _LOGGER_H
#define _LOGGER_H

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <memory>
#include <filesystem>

class LoggerWithTag : public spdlog::logger {
public:
    LoggerWithTag(const std::string& tag, spdlog::sinks_init_list list);
    ~LoggerWithTag() = default;
    static std::shared_ptr<LoggerWithTag> GetLogger(std::string tag);
protected:
    static std::string getTimeStampedLogFile();
private:
};

#endif
