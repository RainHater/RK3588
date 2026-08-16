#include "Application.h"

#include <iostream>
#include <string_view>
#include <utility>

namespace rkplatform::app {

bool Application::HandleVersionOption(
    int argc,
    char* argv[],
    std::string_view version
)
{
    if (argc != 2 || std::string_view(argv[1]) != "--version") {
        return false;
    }

    std::cout << version << '\n';
    return true;
}

Application::Application(
    spdlog::logger& logger,
    std::string version,
    std::filesystem::path executable_directory
)
    : m_logger(logger)
    , m_version(std::move(version))
    , m_executable_directory(std::move(executable_directory))
{}

int Application::Run() {
    m_logger.info("程序版本: {}", m_version);
    m_logger.info("运行路径: {}", m_executable_directory.string());

    return 0;
}

}  // namespace rkplatform::app
