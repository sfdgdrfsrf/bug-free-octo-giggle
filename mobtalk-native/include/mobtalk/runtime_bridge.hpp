#pragma once
#include <atomic>
#include <cstdint>

namespace mobtalk::runtime {
struct Snapshot {
    std::uint64_t localPlayerTicks{};
    std::uint64_t gameModeActions{};
    std::uint64_t attackEvents{};
    bool providerAvailable{};
    bool subscribed{};
};

// Optional runtime bridge to BedrockTools' documented ApiV1 C ABI.
// No Minecraft object pointers are retained beyond the event callback.
bool start() noexcept;
void stop() noexcept;
Snapshot snapshot() noexcept;
}
