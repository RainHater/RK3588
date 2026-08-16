#ifndef RKPLATFORM_APP_APPLICATION_H
#define RKPLATFORM_APP_APPLICATION_H

#include "Logger.h"

#include <filesystem>
#include <string>
#include <string_view>

namespace rkplatform::app {

class Application final {
public:
    // 识别并输出版本参数。
    static bool HandleVersionOption(
        int argc,
        char* argv[],
        std::string_view version
    );

    // 创建应用流程对象。
    Application(
        spdlog::logger& logger,
        std::string version,
        std::filesystem::path executable_directory
    );

    // 运行应用。
    int Run();

private:
    spdlog::logger& m_logger;
    std::string m_version;
    std::filesystem::path m_executable_directory;
};

}  // namespace rkplatform::app

#endif  // RKPLATFORM_APP_APPLICATION_H
