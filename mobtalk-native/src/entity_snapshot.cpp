#include "mobtalk/entity_snapshot.hpp"
#include <cmath>
#include <cstring>
#include <span>

namespace mobtalk::entities {
namespace {
bool finite(const EntitySample& s) noexcept {
    return std::isfinite(s.x) && std::isfinite(s.y) && std::isfinite(s.z) &&
           std::isfinite(s.yaw) && std::isfinite(s.pitch);
}
void copyText(char* dst, std::size_t capacity, std::string_view src) noexcept {
    const auto n = src.size() < capacity - 1 ? src.size() : capacity - 1;
    if (n) std::memcpy(dst, src.data(), n);
    dst[n] = '\0';
}
}
bool SnapshotStore::publish(std::span<const EntitySample> samples) noexcept {
    if (samples.size() > kMaxEntitiesPerFrame) return false;
    SnapshotFrame next{};
    next.generation = frame_.generation + 1;
    for (const auto& sample : samples) {
        if (!sample.runtimeKey || sample.typeId.empty() ||
            sample.typeId.size() >= kTypeIdCapacity ||
            sample.displayName.size() >= kNameCapacity || !finite(sample)) continue;
        bool duplicate = false;
        for (std::size_t i = 0; i < next.count; ++i) {
            if (next.entities[i].runtimeKey == sample.runtimeKey) {
                duplicate = true;
                break;
            }
        }
        if (duplicate) continue;
        auto& out = next.entities[next.count++];
        out.runtimeKey = sample.runtimeKey;
        copyText(out.typeId, sizeof(out.typeId), sample.typeId);
        copyText(out.displayName, sizeof(out.displayName), sample.displayName);
        out.x = sample.x; out.y = sample.y; out.z = sample.z;
        out.yaw = sample.yaw; out.pitch = sample.pitch;
        out.isPlayer = sample.isPlayer; out.isAlive = sample.isAlive;
    }
    frame_ = next;
    return true;
}
SnapshotFrame SnapshotStore::copy() const noexcept { return frame_; }
void SnapshotStore::clear() noexcept {
    const auto nextGeneration = frame_.generation + 1;
    frame_ = SnapshotFrame{};
    frame_.generation = nextGeneration;
}
} // namespace mobtalk::entities
