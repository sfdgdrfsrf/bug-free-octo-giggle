#include "mobtalk/entity_snapshot.hpp"
#include <array>
#include <cassert>
#include <cmath>
#include <limits>

using namespace mobtalk::entities;
int main() {
    SnapshotStore store;
    const std::array<EntitySample, 4> samples{{
        {1, "minecraft:pig", "Pig", 1, 2, 3, 90, 0, false, true},
        {2, "minecraft:zombie", "Zombie", 4, 5, 6, 180, 10, false, true},
        {2, "minecraft:duplicate", "Duplicate", 0, 0, 0, 0, 0, false, true},
        {3, "minecraft:bad", "Bad", std::numeric_limits<float>::quiet_NaN(), 0, 0, 0, 0, false, true}
    }};
    assert(store.publish(samples));
    auto frame = store.copy();
    assert(frame.count == 2);
    assert(frame.entities[0].runtimeKey == 1);
    assert(frame.entities[0].typeId[0] == 'm');
    assert(frame.entities[1].runtimeKey == 2);
    assert(frame.generation == 1);
    std::array<EntitySample, kMaxEntitiesPerFrame + 1> tooMany{};
    assert(!store.publish(tooMany));
    assert(store.copy().generation == 1);
    store.clear();
    assert(store.copy().count == 0 && store.copy().generation == 2);
    return 0;
}
