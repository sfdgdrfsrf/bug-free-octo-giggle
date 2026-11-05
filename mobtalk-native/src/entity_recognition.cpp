#include "mobtalk/entity_recognition.hpp"
#include <mutex>
namespace mobtalk::entities {
namespace {
std::mutex gMutex;
SnapshotStore gSnapshots;
EntityTracker gTracker;
}
bool ingestVerifiedFrame(std::span<const EntitySample> samples,float x,float y,float z,float radius) noexcept {
    std::lock_guard lock(gMutex);
    if(!gSnapshots.publish(samples)) return false;
    gTracker.update(gSnapshots.copy(),x,y,z,radius);
    return true;
}
NearbyFrame nearbyCreatures() noexcept {
    std::lock_guard lock(gMutex);
    return gTracker.nearby();
}
bool findCreature(std::uint64_t id,TrackedCreature& out) noexcept {
    std::lock_guard lock(gMutex);
    return gTracker.find(id,out);
}
void resetRecognition() noexcept {
    std::lock_guard lock(gMutex);
    gSnapshots.clear();
    gTracker.reset();
}
} // namespace mobtalk::entities
