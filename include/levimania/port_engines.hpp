#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace levimania::ports {

// 1. Personalized: deterministic, stable choices from world/player/feature identity.
std::uint64_t stableHash(std::string_view text, std::uint64_t seed = 1469598103934665603ULL) noexcept;
std::uint32_t personalizedVariant(std::string_view worldId, std::string_view playerId,
                                  std::string_view featureId, std::uint32_t variantCount) noexcept;

// 2. Baritone-style weighted navigation primitives. Costs are caller-supplied from safe world queries.
enum class MoveKind : std::uint8_t { Walk, Sprint, Jump, Parkour, Ascend, Descend, Ladder, Water, Door, Hazard };
struct MovementCosts {
    float walk{1.0f}, sprint{0.8f}, jump{1.4f}, parkour{2.2f}, ascend{2.0f};
    float descend{1.1f}, ladder{2.5f}, water{3.0f}, door{1.8f}, hazard{1000.0f};
};
float movementCost(MoveKind kind, const MovementCosts& costs) noexcept;

// 3. AdaptiveMobs: records observable player actions; no game-world mutation occurs here.
enum class PlayerAction : std::uint8_t { Craft, Mine, PlaceBlock, Fight, Flee, Build, Count };
struct ActionMemory { std::array<std::uint32_t, static_cast<std::size_t>(PlayerAction::Count)> counts{}; };
void observeAction(ActionMemory& memory, PlayerAction action) noexcept;
PlayerAction preferredAdaptation(const ActionMemory& memory) noexcept;
float adaptationConfidence(const ActionMemory& memory) noexcept;

// 4. Capri Chip: deterministic parser for supported offline companion requests.
enum class CompanionCommand : std::uint8_t { Unknown, Follow, Stay, FindDiamonds, DigTunnel, BuildWall, PlaceLights, Attack };
CompanionCommand parseCompanionCommand(std::string_view request) noexcept;

// 5. JJK: original technique metadata framework; values are balancing placeholders, not copied assets.
struct TechniqueSpec { std::string_view id; float energyCost; float cooldownSeconds; std::uint8_t grade; };
const std::array<TechniqueSpec, 14>& cursedTechniques() noexcept;

// 6. Jim's Mine: extraction loop state machine.
enum class MinePhase : std::uint8_t { Hub, Preparing, Exploring, Extracting, Failed, Completed };
struct MineRun {
    MinePhase phase{MinePhase::Hub};
    std::uint8_t level{1};
    std::uint32_t copper{0};
    std::uint32_t lootValue{0};
    std::uint32_t wallet{0};
    std::uint32_t gearTier{0};
    std::uint32_t threat{0};
};
bool startMineRun(MineRun& run) noexcept;
void collectMineLoot(MineRun& run, std::uint32_t copper, std::uint32_t lootValue) noexcept;
bool extractMineRun(MineRun& run) noexcept;
bool returnToMineHub(MineRun& run) noexcept;
bool failMineRun(MineRun& run) noexcept;
bool purchaseMineGear(MineRun& run, std::uint32_t cost) noexcept;
bool unlockNextMineLevel(MineRun& run) noexcept;

} // namespace levimania::ports
