#include "Logger.h"



LoggerWithTag::LoggerWithTag(const std::string& tag, spdlog::sinks_init_list list)
    : spdlog::logger(tag, list)
{
    this->set_pattern("[%T] [%n] [%^%l%$] %v"); // 时间 [tag] [LEVEL] 消息
    this->set_level(spdlog::level::debug);
}

std::shared_ptr<LoggerWithTag> LoggerWithTag::GetLogger(std::string tag){
    static std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    static std::shared_ptr<spdlog::sinks::basic_file_sink_mt> file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
        getTimeStampedLogFile(), true
    );

    auto logger = std::make_shared<LoggerWithTag>(
        tag,
        spdlog::sinks_init_list{console_sink, file_sink}
    );

    spdlog::register_logger(logger);

    return logger;
}

std::string LoggerWithTag::getTimeStampedLogFile(){
    auto t = std::time(nullptr);
    std::tm tm{};
    localtime_r(&t, &tm); // Linux 安全版本

    //文件夹：logs/YYYYMMDD
    std::ostringstream folder;
    folder << "logs/"
           << std::put_time(&tm, "%Y-%m-%d");

    //创建文件夹（如果不存在）
    std::filesystem::create_directories(folder.str());

    // 文件名：MMDD-HH.log
    std::ostringstream filename;
    filename << std::put_time(&tm, "%m-%d-%H") << ".log";

    return folder.str() + "/" + filename.str();
}
