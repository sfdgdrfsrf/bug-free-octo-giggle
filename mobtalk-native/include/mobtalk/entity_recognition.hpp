#pragma once
#include "mobtalk/entity_tracker.hpp"
#include <span>
namespace mobtalk::entities {
// Called only by a future verified game-thread adapter. Samples must be copied
// from live runtime objects before calling; no runtime pointers cross this API.
bool ingestVerifiedFrame(std::span<const EntitySample> samples,
                         float playerX,float playerY,float playerZ,float radius) noexcept;
NearbyFrame nearbyCreatures() noexcept;
bool findCreature(std::uint64_t sessionId,TrackedCreature& out) noexcept;
void resetRecognition() noexcept;
} // namespace mobtalk::entities
