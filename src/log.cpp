#include "log.h"

#include <chrono>
#include <cstring>
#include <filesystem>

static std::string_view basename(std::string_view path) {
    auto pos = path.find_last_of("/\\");
    return pos == std::string_view::npos ? path : path.substr(pos + 1);
}

logger& logger::instance() {
    static logger inst;
    return inst;
}

logger::logger() {
    namespace fs = std::filesystem;

    constexpr const char LOG_FILENAME[] = "eol.log";
    constexpr uintmax_t MAX_LOG_FILE_SIZE = 5 * 1024 * 1024; // 5 MiB
    constexpr size_t MAX_KEPT_LOG_FILES = 5;
    std::error_code ec;

    uintmax_t size = fs::file_size(LOG_FILENAME, ec);
    if (!ec && size >= MAX_LOG_FILE_SIZE) {
        fs::remove(std::format("{}.{}", LOG_FILENAME, MAX_KEPT_LOG_FILES), ec);
        for (size_t i = MAX_KEPT_LOG_FILES - 1; i > 0; --i) {
            fs::rename(std::format("{}.{}", LOG_FILENAME, i),
                       std::format("{}.{}", LOG_FILENAME, i + 1), ec);
        }
        fs::rename(LOG_FILENAME, std::format("{}.1", LOG_FILENAME), ec);
    }

    file = std::fopen(LOG_FILENAME, "a");
}

logger::~logger() {
    if (file) {
        std::fclose(file);
    }
}

void logger::write(LogLevel lvl, std::source_location loc, std::string_view msg) {
    std::lock_guard lock(mutex);

    auto now = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
    auto line = std::format("[{:%H:%M:%S}] [{}] {}:{}: {}\n", now, lvl, basename(loc.file_name()),
                            loc.line(), msg);

    if (file) {
        std::fputs(line.c_str(), file);
        if (lvl >= LogLevel::Warn) {
            std::fflush(file);
        }
    }

#ifdef DEBUG
    std::fputs(line.c_str(), stdout);
    std::fflush(stdout);
#endif
}
