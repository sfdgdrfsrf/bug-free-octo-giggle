#include "levimania/port_engines.hpp"
#include <algorithm>
#include <cctype>
#include <limits>
#include <queue>
#include <utility>

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


RouteResult planWeightedRoute(const std::vector<RouteNode>& nodes, const std::vector<RouteEdge>& edges,
                              std::uint16_t start, std::uint16_t goal, std::size_t maxNodes) {
    RouteResult result;
    const std::size_t limit = std::min<std::size_t>(maxNodes, 4096);
    if (nodes.empty() || nodes.size() > limit || start >= nodes.size() || goal >= nodes.size()) return result;
    std::vector<std::vector<std::pair<std::uint16_t,float>>> graph(nodes.size());
    for (const auto& edge : edges) {
        if (edge.from >= nodes.size() || edge.to >= nodes.size() ||
            !(edge.cost >= 0.0f) || edge.cost > 100000.0f) continue;
        graph[edge.from].push_back({edge.to, edge.cost});
    }
    const float infinity = std::numeric_limits<float>::infinity();
    std::vector<float> distance(nodes.size(), infinity);
    std::vector<std::uint16_t> previous(nodes.size(), std::numeric_limits<std::uint16_t>::max());
    using Entry = std::pair<float,std::uint16_t>;
    std::priority_queue<Entry,std::vector<Entry>,std::greater<Entry>> open;
    distance[start]=0.0f; open.push({0.0f,start});
    std::size_t visited=0;
    while (!open.empty() && visited < limit) {
        const auto [cost,current]=open.top(); open.pop();
        if (cost != distance[current]) continue;
        ++visited;
        if (current == goal) break;
        for (const auto& [next,weight] : graph[current]) {
            const float candidate=cost+weight;
            if (candidate < distance[next]) {
                distance[next]=candidate; previous[next]=current; open.push({candidate,next});
            }
        }
    }
    if (!std::isfinite(distance[goal])) return result;
    std::vector<std::uint16_t> reverse;
    for (std::uint16_t at=goal;;at=previous[at]) {
        reverse.push_back(at);
        if (at==start) break;
        if (previous[at] == std::numeric_limits<std::uint16_t>::max() || reverse.size()>limit) return {};
    }
    result.nodes.assign(reverse.rbegin(),reverse.rend());
    result.totalCost=distance[goal]; result.found=true;
    return result;
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


bool enqueueCompanionTask(CompanionTaskQueue& queue, CompanionCommand command) noexcept {
    if (command == CompanionCommand::Unknown || queue.count >= queue.tasks.size()) return false;
    queue.tasks[queue.count++] = command;
    return true;
}
CompanionCommand dequeueCompanionTask(CompanionTaskQueue& queue) noexcept {
    if (queue.count == 0) return CompanionCommand::Unknown;
    const auto result=queue.tasks[0];
    for (std::size_t i=1;i<queue.count;++i) queue.tasks[i-1]=queue.tasks[i];
    --queue.count;
    return result;
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


void tickTechniqueCooldowns(TechniqueRuntime& runtime, float deltaSeconds) noexcept {
    const float dt = std::isfinite(deltaSeconds) ? std::clamp(deltaSeconds,0.0f,10.0f) : 0.0f;
    runtime.energy = std::isfinite(runtime.energy) ? std::clamp(runtime.energy,0.0f,100.0f) : 0.0f;
    for (auto& cooldown : runtime.cooldowns)
        cooldown = std::isfinite(cooldown) ? std::max(0.0f,cooldown-dt) : 0.0f;
}
bool castTechnique(TechniqueRuntime& runtime, std::size_t techniqueIndex) noexcept {
    const auto& techniques=cursedTechniques();
    if (techniqueIndex >= techniques.size()) return false;
    tickTechniqueCooldowns(runtime,0.0f);
    const auto& spec=techniques[techniqueIndex];
    if (runtime.cooldowns[techniqueIndex] > 0.0f || runtime.energy < spec.energyCost) return false;
    runtime.energy -= spec.energyCost;
    runtime.cooldowns[techniqueIndex] = spec.cooldownSeconds;
    return true;
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
bool returnToMineHub(MineRun& run) noexcept {
    if (run.phase != MinePhase::Completed && run.phase != MinePhase::Failed) return false;
    run.phase = MinePhase::Hub;
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
    if ((run.phase != MinePhase::Hub && run.phase != MinePhase::Completed) || cost == 0 || run.wallet < cost) return false;
    run.wallet -= cost;
    saturatingAdd(run.gearTier, 1);
    return true;
}
bool unlockNextMineLevel(MineRun& run) noexcept {
    if ((run.phase != MinePhase::Hub && run.phase != MinePhase::Completed) || run.level >= 5 || run.gearTier < run.level) return false;
    ++run.level;
    return true;
}
} // namespace levimania::ports
