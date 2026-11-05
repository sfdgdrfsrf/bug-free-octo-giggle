#pragma once
#include <atomic>
#include <cstdint>
namespace mobtalk::runtime {
struct Snapshot {
    std::uint64_t localPlayerTicks{};
    std::uint64_t gameModeActions{};
    std::uint64_t attackEvents{};
    std::uint64_t blockBreakActions{};
    std::uint64_t blockPlaceActions{};
    std::uint64_t interactActions{};
    std::uint64_t itemUseActions{};
    bool bedrockToolsAvailable{};
    bool subscribed{};
    bool actorManagerListSignatureAvailable{};
    bool actorEnumerationEnabled{};
    std::uint64_t actorEnumerationCalls{};
    std::uint64_t lastActorCount{};
    std::uint64_t actorEnumerationFailures{};
};
bool start() noexcept;
void setActorEnumerationEnabled(bool enabled) noexcept;
void stop() noexcept;
Snapshot snapshot() noexcept;
}
