#include "levimania/config.hpp"

#include <cstdlib>
#include <system_error>

namespace levimania::core {
Config& Config::instance() {
    static Config config;
    return config;
}

void Config::initialize() {
    if (const char* home = std::getenv("HOME"); home != nullptr) {
        directory_ = std::filesystem::path(home) / "LeviMania";
    } else {
        directory_ = std::filesystem::path("/sdcard/LeviMania");
    }

    std::error_code error;
    std::filesystem::create_directories(directory_, error);
}

const std::filesystem::path& Config::directory() const noexcept {
    return directory_;
}
}
