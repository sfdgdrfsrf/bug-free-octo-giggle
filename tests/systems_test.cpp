#include "levimania/systems.hpp"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

using namespace levimania::systems;

int main() {
    {
        std::vector<std::uint8_t> blocked(25, 0);
        auto path = findPath(blocked, {5, 5, 100}, {0, 0}, {4, 4});
        assert(!path.empty() && path.front().x == 0 && path.front().z == 0);
        assert(path.back().x == 4 && path.back().z == 4);
        assert(path.size() == 9);
    }
    {
        std::vector<std::uint8_t> blocked(9, 0);
        blocked[1] = blocked[3] = 1;
        assert(findPath(blocked, {3, 3, 100}, {0, 0}, {2, 2}).empty());
        assert(findPath(blocked, {3, 3, 100}, {-1, 0}, {2, 2}).empty());
        assert(findPath(blocked, {0, 3, 100}, {0, 0}, {0, 2}).empty());
        assert(findPath(blocked, {1024, 1024, 100}, {0, 0}, {1, 1}).empty());
    }
    {
        auto adaptation = adaptMob(-10.0f, 0.0f, 0.0f, 1.0f);
        assert(std::isfinite(adaptation.aggression));
        auto bad = adaptMob(std::numeric_limits<float>::quiet_NaN(), 0, 0, 1);
        assert(std::isfinite(bad.damageScale));
    }
    {
        CompanionState companion;
        updateCompanion(companion, std::numeric_limits<float>::quiet_NaN(), true);
        assert(std::isfinite(companion.stamina) && companion.stamina <= 100.0f);
        updateCompanion(companion, 100000.0f, false);
        assert(companion.stamina >= 0.0f);
    }
    {
        CursedEnergy energy;
        assert(spendEnergy(energy, 25.0f));
        assert(!spendEnergy(energy, -1.0f));
        assert(!spendEnergy(energy, std::numeric_limits<float>::quiet_NaN()));
        regenerateEnergy(energy, 5.0f);
        assert(energy.current <= energy.maximum);
        Technique technique{"test", 10.0f, 1.0f};
        assert(canUseTechnique(energy, technique, 0.0f));
        assert(!canUseTechnique(energy, technique, 1.0f));
    }
    {
        MineEncounter encounter;
        updateMineEncounter(encounter, 1.0f, 1.0f, 1.0f, 10.0f);
        assert(encounter.danger >= 0.65f);
        assert(shouldRevealMimic(encounter));
        updateMineEncounter(encounter, std::numeric_limits<float>::quiet_NaN(), 0, 0, -1);
        assert(std::isfinite(encounter.danger));
    }
    return 0;
}
