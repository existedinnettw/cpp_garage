#include "log.hpp"

int main() {
    app::init_logging();
    app::info("zephyr_conan_sysbuild on {} (Zephyr {}), logging via {}", CONFIG_BOARD_TARGET,
              KERNEL_VERSION_STRING, app::backend());

    for (unsigned tick = 0;; ++tick) {
        app::info("tick {:>4}  uptime {:>8} ms", tick, k_uptime_get());
        k_sleep(K_SECONDS(1));
    }
    return 0;
}
