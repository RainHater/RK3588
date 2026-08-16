#include "Logger.h"

#include <spdlog/spdlog.h>

#include <array>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <thread>
#include <vector>

int main() {
    bool empty_path_rejected = false;
    try {
        rkplatform::component::logging::SetLogPath("");
    } catch (const std::invalid_argument&) {
        empty_path_rejected = true;
    }
    if (!empty_path_rejected) {
        return 1;
    }

    const auto log_file =
        std::filesystem::temp_directory_path() /
        "rkplatform_logger_test" /
        "custom.log";
    std::error_code remove_error;
    std::filesystem::remove(log_file, remove_error);
    rkplatform::component::logging::SetLogPath(log_file.string());

    const auto first =
        rkplatform::component::logging::GetLogger("logger_namespace_test");
    const auto second =
        rkplatform::component::logging::GetLogger("logger_namespace_test");
    const auto different =
        rkplatform::component::logging::GetLogger("logger_namespace_test_2");

    if (first == nullptr || second == nullptr || different == nullptr) {
        return 1;
    }

    if (first != second || first == different) {
        return 1;
    }

    first->info("custom log path test");
    first->flush();
    if (!std::filesystem::exists(log_file)) {
        return 1;
    }

    bool late_path_change_rejected = false;
    try {
        rkplatform::component::logging::SetLogPath("another.log");
    } catch (const std::logic_error&) {
        late_path_change_rejected = true;
    }
    if (!late_path_change_rejected) {
        return 1;
    }

    constexpr std::size_t kThreadCount = 16;
    std::array<std::shared_ptr<spdlog::logger>, kThreadCount> concurrent_loggers;
    std::vector<std::thread> threads;
    threads.reserve(kThreadCount);
    for (std::size_t i = 0; i < kThreadCount; ++i) {
        threads.emplace_back([i, &concurrent_loggers]() {
            concurrent_loggers[i] =
                rkplatform::component::logging::GetLogger(
                    "logger_concurrent_test"
                );
        });
    }
    for (auto& thread : threads) {
        thread.join();
    }
    for (const auto& logger : concurrent_loggers) {
        if (logger != concurrent_loggers.front()) {
            return 1;
        }
    }

    const auto external_logger =
        std::make_shared<spdlog::logger>("logger_external_conflict_test");
    spdlog::register_logger(external_logger);

    bool conflict_rejected = false;
    try {
        rkplatform::component::logging::GetLogger(
            "logger_external_conflict_test"
        );
    } catch (const std::runtime_error&) {
        conflict_rejected = true;
    }
    spdlog::drop("logger_external_conflict_test");

    return conflict_rejected ? 0 : 1;
}
