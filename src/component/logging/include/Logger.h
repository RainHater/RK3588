#ifndef RKPLATFORM_COMPONENT_LOGGER_H
#define RKPLATFORM_COMPONENT_LOGGER_H

#include <spdlog/logger.h>

#include <memory>
#include <string>

namespace rkplatform::component::logging {

// 设置日志文件路径，需在首次获取日志器前调用。
void SetLogPath(const std::string& log_file_path);

// 获取或创建指定标签的日志器。
std::shared_ptr<spdlog::logger> GetLogger(const std::string& tag);

}  // namespace rkplatform::component::logging

#endif  // RKPLATFORM_COMPONENT_LOGGER_H
