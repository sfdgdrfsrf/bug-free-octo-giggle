#pragma once
#include "mobtalk/entity_snapshot.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

namespace mobtalk::entities {
inline constexpr std::size_t kMaxNearbyCreatures = 32;
struct TrackedCreature {
    std::uint64_t sessionId{};
    std::uint64_t runtimeKey{}; // transient only; never serialize
    char typeId[kTypeIdCapacity]{};
    char displayName[kNameCapacity]{};
    float x{}, y{}, z{}, yaw{}, pitch{}, distanceSquared{};
    std::uint32_t lastSeenGeneration{};
    bool isPlayer{};
    bool isAlive{};
};
struct NearbyFrame {
    std::array<TrackedCreature, kMaxNearbyCreatures> creatures{};
    std::size_t count{};
    std::uint64_t generation{};
};
class EntityTracker {
public:
    // Player coordinates must be sampled on the game thread by a verified adapter.
    void update(const SnapshotFrame& frame, float playerX, float playerY, float playerZ,
                float radius) noexcept;
    NearbyFrame nearby() const noexcept;
    bool find(std::uint64_t sessionId, TrackedCreature& out) const noexcept;
    void reset() noexcept;
private:
    std::array<TrackedCreature, kMaxNearbyCreatures> tracked_{};
    std::size_t count_{};
    std::uint64_t nextSessionId_{1};
    std::uint64_t generation_{};
};
} // namespace mobtalk::entities
