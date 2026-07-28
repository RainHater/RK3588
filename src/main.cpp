#include "Logger.h"
#include "Tools.h"

#include <filesystem>
#include <iostream>
#include <string_view>

#ifndef APP_VERSION
#define APP_VERSION "0.0.0"
#endif

int main(int argc, char* argv[]){
    if (argc == 2) {
        const std::string_view option(argv[1]);
        if (option == "--version") {
            std::cout << APP_VERSION << '\n';
            return 0;
        }
    }

    auto root_dir = Tools::GetExecutableDirectory();
    auto logger = LoggerWithTag::GetLogger("main");

    logger->info("程序版本: {}", APP_VERSION);
    logger->info("运行路径: {}", root_dir.string());

    return 0;
}
