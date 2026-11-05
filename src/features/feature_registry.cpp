#include "levimania/feature_registry.hpp"

namespace levimania::features {
const std::vector<FeatureInfo>& planned_features() {
    static const std::vector<FeatureInfo> features{
        {"world_personalization", "World Personalization", "core-logic"},
        {"pathfinding", "Pathfinding and Mining", "core-logic"},
        {"adaptive_mobs", "Adaptive Mob Behaviors", "core-logic"},
        {"companions", "Companions", "core-logic"},
        {"cursed_combat", "Cursed Combat Techniques", "core-logic"},
        {"mining_encounters", "Mining Encounters", "core-logic"}
    };
    return features;
}
}
