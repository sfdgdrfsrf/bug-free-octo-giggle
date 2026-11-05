#include "levimania/port_engines.hpp"
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>

using namespace levimania::ports;

int main() {
    // Personalized: stable for same identity, changes with player/feature, safe for zero variants.
    const auto a = personalizedVariant("world-a", "player-a", "mob", 7);
    assert(a == personalizedVariant("world-a", "player-a", "mob", 7));
    assert(a < 7);
    assert(personalizedVariant("world-a", "player-a", "mob", 0) == 0);

    // Baritone movement costs: hazard penalty dominates and invalid costs fall back safely.
    MovementCosts costs;
    costs.hazard = 5000.0f;
    assert(movementCost(MoveKind::Hazard, costs) > movementCost(MoveKind::Walk, costs));
    costs.walk = std::numeric_limits<float>::infinity();
    assert(movementCost(MoveKind::Walk, costs) == 1.0f);

    // AdaptiveMobs: action memory saturates and chooses the most frequent learned behavior.
    ActionMemory memory;
    assert(preferredAdaptation(memory) == PlayerAction::Count);
    observeAction(memory, PlayerAction::Mine);
    observeAction(memory, PlayerAction::Mine);
    observeAction(memory, PlayerAction::Craft);
    assert(preferredAdaptation(memory) == PlayerAction::Mine);
    assert(std::abs(adaptationConfidence(memory) - (2.0f / 3.0f)) < 0.001f);

    // Capri Chip: offline request parser maps common commands.
    assert(parseCompanionCommand("Please follow me") == CompanionCommand::Follow);
    assert(parseCompanionCommand("find diamonds") == CompanionCommand::FindDiamonds);
    assert(parseCompanionCommand("put some torches here") == CompanionCommand::PlaceLights);
    assert(parseCompanionCommand("do something weird") == CompanionCommand::Unknown);

    // JJK: 14 distinct stable identifiers with non-negative costs/cooldowns.
    const auto& techniques = cursedTechniques();
    assert(techniques.size() == 14);
    for (const auto& technique : techniques) {
        assert(!technique.id.empty());
        assert(technique.energyCost >= 0.0f);
        assert(technique.cooldownSeconds >= 0.0f);
    }

    // Jim's Mine: descend, loot, extract, upgrade, unlock, and start the next run.
    MineRun run;
    assert(startMineRun(run));
    assert(!startMineRun(run));
    collectMineLoot(run, 30, 20);
    assert(run.threat == 1);
    assert(extractMineRun(run));
    assert(run.wallet == 50);
    assert(purchaseMineGear(run, 20));
    assert(run.gearTier == 1 && run.wallet == 30);
    assert(unlockNextMineLevel(run));
    assert(run.level == 2);
    assert(returnToMineHub(run));
    assert(startMineRun(run));
    assert(failMineRun(run));
    assert(run.copper == 0 && run.lootValue == 0);
    assert(returnToMineHub(run));
    return 0;
}
