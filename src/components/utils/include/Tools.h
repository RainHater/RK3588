#ifndef TOOLS_H
#define TOOLS_H

#include <filesystem>
#include <system_error>

namespace Tools {

// 获取当前运行程序的完整路径，例如 /opt/app/bin/app
inline std::filesystem::path GetExecutablePath()
{
    std::error_code error;
    const std::filesystem::path path =
        std::filesystem::read_symlink("/proc/self/exe", error);

    return error ? std::filesystem::path{} : path;
}

// 获取当前运行程序所在目录，例如 /opt/app/bin
inline std::filesystem::path GetExecutableDirectory()
{
    const std::filesystem::path path = GetExecutablePath();
    return path.empty() ? std::filesystem::path{} : path.parent_path();
}

} 

#endif  // TOOLS_H
