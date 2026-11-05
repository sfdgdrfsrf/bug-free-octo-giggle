#include "levimania/feature_registry.hpp"

namespace levimania::features {
const std::vector<FeatureInfo>& planned_features() {
    static const std::vector<FeatureInfo> features{
        {"world_personalization", "World Personalization", "planned"},
        {"pathfinding", "Pathfinding and Mining", "planned"},
        {"adaptive_mobs", "Adaptive Mob Behaviors", "planned"},
        {"companions", "Companions", "planned"},
        {"cursed_combat", "Cursed Combat Techniques", "planned"},
        {"mining_encounters", "Mining Encounters", "planned"}
    };
    return features;
}
}
