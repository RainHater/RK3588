#ifndef _LOGGER_H
#define _LOGGER_H

#include <memory>
#include <string>
#include <spdlog/logger.h>

class Logger {
public:
    /// 初始化
    static void init(const std::string& logDir = "logs");
    /// 获取模块 logger（如 "NET" / "UI" / "DRV"）
    static std::shared_ptr<spdlog::logger> get(const std::string& name);
    /// 默认 logger
    static std::shared_ptr<spdlog::logger> defaultLogger();
    /// 是否已初始化
    static bool isInited();

private:
    static void createDefaultLogger(const std::string& logDir);

private:
    static bool s_inited;
};

#endif
