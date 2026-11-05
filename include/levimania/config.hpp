#pragma once

#include <filesystem>

namespace levimania::core {
class Config {
public:
    static Config& instance();
    void initialize();
    [[nodiscard]] const std::filesystem::path& directory() const noexcept;
private:
    std::filesystem::path directory_;
};
}
