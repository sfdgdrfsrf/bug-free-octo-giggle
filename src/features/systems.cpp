#include "levimania/systems.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>

namespace levimania::systems {
namespace {
constexpr int kMaxGridWidth = 1024;
constexpr int kMaxGridHeight = 1024;
constexpr std::size_t kMaxGridCells = 262144;

float finiteOr(float value, float fallback) {
    return std::isfinite(value) ? value : fallback;
}

int indexOf(GridPoint point, int width) {
    return point.z * width + point.x;
}

int heuristic(GridPoint a, GridPoint b) {
    return std::abs(a.x - b.x) + std::abs(a.z - b.z);
}
} // namespace

std::vector<GridPoint> findPath(const std::vector<std::uint8_t>& blocked,
                                PathOptions options, GridPoint start, GridPoint goal) {
    const std::vector<GridPoint> noPath;
    if (options.width <= 0 || options.height <= 0 ||
        options.width > kMaxGridWidth || options.height > kMaxGridHeight) {
        return noPath;
    }

    const auto width = static_cast<std::size_t>(options.width);
    const auto height = static_cast<std::size_t>(options.height);
    const std::size_t cellCount = width * height;
    if (cellCount > kMaxGridCells || blocked.size() < cellCount ||
        start.x < 0 || start.z < 0 || goal.x < 0 || goal.z < 0 ||
        start.x >= options.width || goal.x >= options.width ||
        start.z >= options.height || goal.z >= options.height) {
        return noPath;
    }

    const int startIndex = indexOf(start, options.width);
    const int goalIndex = indexOf(goal, options.width);
    if (blocked[static_cast<std::size_t>(startIndex)] != 0 ||
        blocked[static_cast<std::size_t>(goalIndex)] != 0) {
        return noPath;
    }

    struct Node { int index; int score; };
    const auto compare = [](const Node& a, const Node& b) { return a.score > b.score; };
    std::priority_queue<Node, std::vector<Node>, decltype(compare)> open(compare);
    std::vector<int> parent(cellCount, -1);
    std::vector<int> cost(cellCount, std::numeric_limits<int>::max());
    cost[static_cast<std::size_t>(startIndex)] = 0;
    open.push({startIndex, heuristic(start, goal)});

    const int maxVisited = std::clamp(options.maxVisited, 1, static_cast<int>(cellCount));
    int visited = 0;
    constexpr int dx[] = {1, -1, 0, 0};
    constexpr int dz[] = {0, 0, 1, -1};

    while (!open.empty() && visited < maxVisited) {
        const Node current = open.top();
        open.pop();
        if (current.index == goalIndex) {
            std::vector<GridPoint> path;
            int at = goalIndex;
            while (at != startIndex) {
                if (at < 0 || static_cast<std::size_t>(at) >= parent.size()) return noPath;
                path.push_back({at % options.width, at / options.width});
                at = parent[static_cast<std::size_t>(at)];
            }
            path.push_back(start);
            std::reverse(path.begin(), path.end());
            return path;
        }

        ++visited;
        const GridPoint point{current.index % options.width, current.index / options.width};
        for (int direction = 0; direction < 4; ++direction) {
            const GridPoint next{point.x + dx[direction], point.z + dz[direction]};
            if (next.x < 0 || next.z < 0 ||
                next.x >= options.width || next.z >= options.height) continue;

            const int nextIndex = indexOf(next, options.width);
            const auto nextOffset = static_cast<std::size_t>(nextIndex);
            if (blocked[nextOffset] != 0 || cost[nextOffset] <= cost[static_cast<std::size_t>(current.index)] + 1) continue;

            cost[nextOffset] = cost[static_cast<std::size_t>(current.index)] + 1;
            parent[nextOffset] = current.index;
            open.push({nextIndex, cost[nextOffset] + heuristic(next, goal)});
        }
    }
    return noPath;
}

MobAdaptation adaptMob(float deaths, float kills, float damage, float difficulty) {
    deaths = std::max(0.0f, finiteOr(deaths, 0.0f));
    kills = std::max(0.0f, finiteOr(kills, 0.0f));
    damage = std::max(0.0f, finiteOr(damage, 0.0f));
    difficulty = finiteOr(difficulty, 1.0f);
    const float pressure = std::clamp((deaths * 0.8f + kills * 0.15f + damage * 0.02f) / 10.0f, 0.0f, 1.0f);
    const float tier = std::clamp(difficulty, 0.25f, 2.0f);
    return {
        std::clamp(tier * (0.85f + pressure * 0.3f), 0.5f, 2.5f),
        std::clamp(1.0f + pressure * 0.25f, 0.75f, 1.5f),
        std::clamp(1.0f + pressure * 0.2f, 0.75f, 1.5f)
    };
}

void updateCompanion(CompanionState& companion, float deltaSeconds, bool nearby) {
    const float dt = std::clamp(finiteOr(deltaSeconds, 0.0f), 0.0f, 10.0f);
    companion.stamina = std::clamp(finiteOr(companion.stamina, 100.0f) +
        (nearby ? 3.0f : -1.5f) * dt, 0.0f, 100.0f);
    if (nearby) companion.bond = std::clamp(finiteOr(companion.bond, 0.0f) + 0.02f * dt, 0.0f, 100.0f);
}

bool spendEnergy(CursedEnergy& energy, float amount) {
    if (!std::isfinite(amount) || amount < 0.0f ||
        !std::isfinite(energy.current) || !std::isfinite(energy.maximum) ||
        energy.maximum < 0.0f || energy.current < amount) return false;
    energy.current = std::clamp(energy.current - amount, 0.0f, energy.maximum);
    return true;
}

void regenerateEnergy(CursedEnergy& energy, float deltaSeconds) {
    if (!std::isfinite(energy.maximum) || energy.maximum < 0.0f) {
        energy.current = 0.0f;
        return;
    }
    const float dt = std::clamp(finiteOr(deltaSeconds, 0.0f), 0.0f, 10.0f);
    const float regen = std::max(0.0f, finiteOr(energy.regenerationPerSecond, 0.0f));
    energy.current = std::clamp(finiteOr(energy.current, 0.0f) + dt * regen, 0.0f, energy.maximum);
}

bool canUseTechnique(const CursedEnergy& energy, const Technique& technique, float cooldownRemaining) {
    return std::isfinite(technique.cost) && technique.cost >= 0.0f &&
           std::isfinite(energy.current) && energy.current >= technique.cost &&
           std::isfinite(cooldownRemaining) && cooldownRemaining <= 0.0f;
}

void updateMineEncounter(MineEncounter& encounter, float depth, float isolation,
                         float noise, float deltaSeconds) {
    const float d = std::clamp(finiteOr(depth, 0.0f), 0.0f, 1.0f);
    const float i = std::clamp(finiteOr(isolation, 0.0f), 0.0f, 1.0f);
    const float n = std::clamp(finiteOr(noise, 0.0f), 0.0f, 1.0f);
    const float dt = std::clamp(finiteOr(deltaSeconds, 0.0f), 0.0f, 10.0f);
    encounter.depthScore = d;
    encounter.danger = std::clamp(d * 0.55f + i * 0.3f + n * 0.15f, 0.0f, 1.0f);
    const float target = std::clamp(d * 0.35f + i * 0.45f + n * 0.2f, 0.0f, 1.0f);
    const float prior = std::clamp(finiteOr(encounter.mimicSuspicion, 0.0f), 0.0f, 1.0f);
    encounter.mimicSuspicion = std::clamp(prior + (target - prior) * std::min(1.0f, dt * 0.15f), 0.0f, 1.0f);
}

bool shouldRevealMimic(const MineEncounter& encounter) {
    return std::isfinite(encounter.mimicSuspicion) && std::isfinite(encounter.danger) &&
           encounter.mimicSuspicion >= 0.82f && encounter.danger >= 0.65f;
}
} // namespace levimania::systems
