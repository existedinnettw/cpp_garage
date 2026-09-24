// Tiny logging facade so main.cpp is identical on every board.
//
// Standard/third-party headers must come before <zephyr/kernel.h>: Zephyr
// defines a `__unused` macro that collides with a struct member of the same
// name in glibc's <bits/struct_mutex.h> on native_sim.
#pragma once

#include <fmt/format.h>

#include <string>
#include <utility>

#if APP_HAVE_SPDLOG
#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/null_mutex.h>
#endif

#include <zephyr/kernel.h>
#include <zephyr/version.h>

namespace app {

#if APP_HAVE_SPDLOG

// spdlog sink that forwards to Zephyr's printk console. This sample logs from
// a single thread and printk already serialises, so a null_mutex suffices.
class printk_sink final : public spdlog::sinks::base_sink<spdlog::details::null_mutex> {
protected:
    void sink_it_(const spdlog::details::log_msg &msg) override {
        spdlog::memory_buf_t formatted;
        formatter_->format(msg, formatted);
        printk("%.*s", static_cast<int>(formatted.size()), formatted.data());
    }
    void flush_() override {}
};

inline void init_logging() {
    auto logger = std::make_shared<spdlog::logger>("app", std::make_shared<printk_sink>());
    logger->set_pattern("[%n] [%^%l%$] %v");
    spdlog::set_default_logger(std::move(logger));
}

template <typename... Args>
void info(fmt::format_string<Args...> fmt_str, Args &&...args) {
    spdlog::info(fmt_str, std::forward<Args>(args)...);
}

inline std::string backend() {
    return fmt::format("spdlog {}.{}.{}", SPDLOG_VER_MAJOR, SPDLOG_VER_MINOR, SPDLOG_VER_PATCH);
}

#else

inline void init_logging() {}

template <typename... Args>
void info(fmt::format_string<Args...> fmt_str, Args &&...args) {
    fmt::memory_buffer buf;
    fmt::format_to(fmt::appender(buf), "[app] [info] ");
    fmt::format_to(fmt::appender(buf), fmt_str, std::forward<Args>(args)...);
    printk("%.*s\n", static_cast<int>(buf.size()), buf.data());
}

inline std::string backend() {
    return fmt::format("fmt {}.{}.{}", FMT_VERSION / 10000, FMT_VERSION / 100 % 100, FMT_VERSION % 100);
}

#endif

} // namespace app
