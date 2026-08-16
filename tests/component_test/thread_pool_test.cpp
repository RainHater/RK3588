#include "ThreadPool.h"

#include <stdexcept>

int main() {
    rkplatform::component::ThreadPool pool(1);
    pool.Start();

    auto result = pool.Enqueue([]() { return 42; });
    if (result.get() != 42) {
        return 1;
    }

    pool.Stop();
    pool.Stop();

    try {
        pool.Start();
    } catch (const std::runtime_error&) {
        return 0;
    }

    return 1;
}
