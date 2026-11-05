#include "levimania/port_engines.hpp"
#include <algorithm>
#include <cctype>
#include <limits>

namespace levimania::ports {
namespace {
float safeCost(float value, float fallback) noexcept {
    return value >= 0.0f && value <= 100000.0f ? value : fallback;
}
std::string lower(std::string_view input) {
    std::string out;
    out.reserve(input.size());
    for (unsigned char c : input) out.push_back(static_cast<char>(std::tolower(c)));
    return out;
}
void saturatingAdd(std::uint32_t& value, std::uint32_t amount) noexcept {
    const auto max = std::numeric_limits<std::uint32_t>::max();
    value = amount > max - value ? max : value + amount;
}
} // namespace

std::uint64_t stableHash(std::string_view text, std::uint64_t seed) noexcept {
    std::uint64_t hash = seed;
    for (unsigned char c : text) {
        hash ^= c;
        hash *= 1099511628211ULL;
    }
    return hash;
}

std::uint32_t personalizedVariant(std::string_view worldId, std::string_view playerId,
                                  std::string_view featureId, std::uint32_t variantCount) noexcept {
    if (variantCount == 0) return 0;
    auto hash = stableHash(worldId);
    hash = stableHash(playerId, hash);
    hash = stableHash(featureId, hash);
    return static_cast<std::uint32_t>(hash % variantCount);
}

float movementCost(MoveKind kind, const MovementCosts& c) noexcept {
    switch (kind) {
        case MoveKind::Walk: return safeCost(c.walk, 1.0f);
        case MoveKind::Sprint: return safeCost(c.sprint, 0.8f);
        case MoveKind::Jump: return safeCost(c.jump, 1.4f);
        case MoveKind::Parkour: return safeCost(c.parkour, 2.2f);
        case MoveKind::Ascend: return safeCost(c.ascend, 2.0f);
        case MoveKind::Descend: return safeCost(c.descend, 1.1f);
        case MoveKind::Ladder: return safeCost(c.ladder, 2.5f);
        case MoveKind::Water: return safeCost(c.water, 3.0f);
        case MoveKind::Door: return safeCost(c.door, 1.8f);
        case MoveKind::Hazard: return safeCost(c.hazard, 1000.0f);
    }
    return 1000.0f;
}

void observeAction(ActionMemory& memory, PlayerAction action) noexcept {
    const auto index = static_cast<std::size_t>(action);
    if (index >= memory.counts.size()) return;
    if (memory.counts[index] != std::numeric_limits<std::uint32_t>::max()) ++memory.counts[index];
}

PlayerAction preferredAdaptation(const ActionMemory& memory) noexcept {
    std::size_t best = 0;
    for (std::size_t i = 1; i < memory.counts.size(); ++i)
        if (memory.counts[i] > memory.counts[best]) best = i;
    return memory.counts[best] == 0 ? PlayerAction::Count : static_cast<PlayerAction>(best);
}

float adaptationConfidence(const ActionMemory& memory) noexcept {
    std::uint64_t total = 0;
    std::uint32_t best = 0;
    for (auto count : memory.counts) {
        total += count;
        best = std::max(best, count);
    }
    return total == 0 ? 0.0f : static_cast<float>(best) / static_cast<float>(total);
}

CompanionCommand parseCompanionCommand(std::string_view request) noexcept {
    const auto s = lower(request);
    if (s.find("follow") != std::string::npos || s.find("come here") != std::string::npos) return CompanionCommand::Follow;
    if (s.find("stay") != std::string::npos || s.find("wait here") != std::string::npos) return CompanionCommand::Stay;
    if (s.find("diamond") != std::string::npos || s.find("ore") != std::string::npos) return CompanionCommand::FindDiamonds;
    if (s.find("tunnel") != std::string::npos || s.find("dig") != std::string::npos) return CompanionCommand::DigTunnel;
    if (s.find("wall") != std::string::npos || s.find("build") != std::string::npos) return CompanionCommand::BuildWall;
    if (s.find("torch") != std::string::npos || s.find("light") != std::string::npos) return CompanionCommand::PlaceLights;
    if (s.find("attack") != std::string::npos || s.find("defend") != std::string::npos) return CompanionCommand::Attack;
    return CompanionCommand::Unknown;
}

const std::array<TechniqueSpec, 14>& cursedTechniques() noexcept {
    static constexpr std::array<TechniqueSpec, 14> techniques{{
        {"impact_burst", 12.0f, 2.0f, 4}, {"reinforced_strike", 18.0f, 4.0f, 4},
        {"cursed_projectile", 22.0f, 5.0f, 3}, {"binding_snare", 28.0f, 8.0f, 3},
        {"shadow_step", 16.0f, 6.0f, 3}, {"barrier_guard", 20.0f, 7.0f, 3},
        {"gravity_well", 35.0f, 12.0f, 2}, {"echo_barrage", 30.0f, 10.0f, 2},
        {"spatial_rend", 42.0f, 14.0f, 2}, {"black_flash", 25.0f, 9.0f, 2},
        {"spirit_surge", 48.0f, 18.0f, 1}, {"domain_shell", 55.0f, 22.0f, 1},
        {"domain_expansion", 80.0f, 45.0f, 1}, {"maximum_technique", 100.0f, 60.0f, 1}
    }};
    return techniques;
}

bool startMineRun(MineRun& run) noexcept {
    if (run.phase != MinePhase::Hub || run.level == 0 || run.level > 5) return false;
    run.phase = MinePhase::Preparing;
    run.copper = 0;
    run.lootValue = 0;
    run.threat = 0;
    run.phase = MinePhase::Exploring;
    return true;
}
void collectMineLoot(MineRun& run, std::uint32_t copper, std::uint32_t lootValue) noexcept {
    if (run.phase != MinePhase::Exploring) return;
    saturatingAdd(run.copper, copper);
    saturatingAdd(run.lootValue, lootValue);
    saturatingAdd(run.threat, 1);
}
bool extractMineRun(MineRun& run) noexcept {
    if (run.phase != MinePhase::Exploring) return false;
    saturatingAdd(run.wallet, run.copper);
    saturatingAdd(run.wallet, run.lootValue);
    run.phase = MinePhase::Completed;
    return true;
}
bool failMineRun(MineRun& run) noexcept {
    if (run.phase != MinePhase::Exploring) return false;
    run.phase = MinePhase::Failed;
    run.copper = 0;
    run.lootValue = 0;
    return true;
}
bool purchaseMineGear(MineRun& run, std::uint32_t cost) noexcept {
    if (run.phase != MinePhase::Hub || cost == 0 || run.wallet < cost) return false;
    run.wallet -= cost;
    saturatingAdd(run.gearTier, 1);
    return true;
}
bool unlockNextMineLevel(MineRun& run) noexcept {
    if (run.phase != MinePhase::Hub || run.level >= 5 || run.gearTier < run.level) return false;
    ++run.level;
    return true;
}
} // namespace levimania::ports
