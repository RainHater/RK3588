#include "Application.h"
#include "Logger.h"
#include "Tools.h"

#ifndef APP_VERSION
#define APP_VERSION "0.0.0"
#endif

int main(int argc, char* argv[]) {
    if (rkplatform::app::Application::HandleVersionOption(
            argc, argv, APP_VERSION)) {
        return 0;
    }

    const auto executable_directory =
        rkplatform::platform::GetExecutableDirectory();
    auto logger = rkplatform::component::logging::GetLogger("main");
    rkplatform::app::Application application(
        *logger,
        APP_VERSION,
        executable_directory
    );

    return application.Run();
}
