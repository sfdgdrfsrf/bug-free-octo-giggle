#pragma once
#include <atomic>
#include <cstdint>
namespace mobtalk::runtime {
struct Snapshot {
    std::uint64_t localPlayerTicks{};
    std::uint64_t gameModeActions{};
    std::uint64_t attackEvents{};
    bool bedrockToolsAvailable{};
    bool subscribed{};
};
bool start() noexcept;
void stop() noexcept;
Snapshot snapshot() noexcept;
}
