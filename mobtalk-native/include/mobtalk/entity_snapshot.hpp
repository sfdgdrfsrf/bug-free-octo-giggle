#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <span>
#include <mutex>

namespace mobtalk::entities {
inline constexpr std::size_t kMaxEntitiesPerFrame = 128;
inline constexpr std::size_t kTypeIdCapacity = 96;
inline constexpr std::size_t kNameCapacity = 64;

// Pointer-free data copied from a runtime adapter on the game thread.
// runtimeKey is transient and must never be persisted as creature identity.
struct EntitySnapshot {
    std::uint64_t runtimeKey{};
    char typeId[kTypeIdCapacity]{};
    char displayName[kNameCapacity]{};
    float x{}, y{}, z{}, yaw{}, pitch{};
    bool isPlayer{};
    bool isAlive{};
};

struct EntitySample {
    std::uint64_t runtimeKey{};
    std::string_view typeId;
    std::string_view displayName;
    float x{}, y{}, z{}, yaw{}, pitch{};
    bool isPlayer{};
    bool isAlive{};
};

struct SnapshotFrame {
    std::array<EntitySnapshot, kMaxEntitiesPerFrame> entities{};
    std::size_t count{};
    std::uint64_t generation{};
};

class SnapshotStore {
public:
    // Rejects oversized frames. Invalid samples are skipped. No input view or
    // runtime pointer is retained after this call returns.
    bool publish(std::span<const EntitySample> samples) noexcept;
    SnapshotFrame copy() const noexcept;
    void clear() noexcept;
private:
    mutable std::mutex mutex_;
    SnapshotFrame frame_{};
};

} // namespace mobtalk::entities
