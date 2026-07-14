#include "Logger.h"
#include "Tools.h"

#ifndef APP_VERSION
#define APP_VERSION "0.0.0"
#endif

int main(){
    auto root_dir = Tools::GetExecutableDirectory();
    auto logger = LoggerWithTag::GetLogger("main");

    logger->info("程序版本: {}", APP_VERSION);
    logger->info("运行路径: {}", root_dir.string());

    return 0;
}
