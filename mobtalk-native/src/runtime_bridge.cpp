#include "mobtalk/runtime_bridge.hpp"
#include "mobtalk/entity_recognition.hpp"

#include <dlfcn.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <string>
#include <vector>

namespace mobtalk::runtime {
namespace {
using EventType = std::uint32_t;
using EventPriority = std::int32_t;
using EventCallback = void(*)(EventType, void*, void*);
struct ApiV1 {
    std::uint32_t abiVersion;
    std::uint32_t structSize;
    std::uintptr_t (*resolveSignature)(std::uint16_t);
    void* (*clientInstance)();
    std::uint64_t (*subscribe)(EventType, EventPriority, EventCallback, void*);
    void (*unsubscribe)(std::uint64_t);
};
using GetApi = const ApiV1*(*)(std::uint32_t);
constexpr std::uint32_t kAbiVersion = 1;
constexpr EventType kLocalPlayerTick = 2;
constexpr EventType kAttack = 4;
constexpr EventType kGameModeAction = 8;
constexpr EventPriority kNormalPriority = 0;

// BedrockTools v1.5.10 SignatureId ordinals. Its 26.60 support landed in
// commit 3de9a7d; the release includes the updated 26.60 offsets/signatures.
constexpr std::uint16_t kActorManagerListSignatureId = 12;
constexpr std::uint16_t kActorIsPlayerSignatureId = 66;
constexpr std::uint16_t kActorGetNameTagSignatureId = 69;

// Minecraft Bedrock Android 26.60 offsets mirrored by BedrockTools v1.5.10.
constexpr std::uintptr_t kPlayerLevelOffset = 0x1F0;
constexpr std::uintptr_t kLevelActorManagerOffset = 0x478;
constexpr std::uintptr_t kActorStateVectorOffset = 0x228;
constexpr std::uintptr_t kActorRotationOffset = 0x238;
constexpr std::size_t kMaxRuntimeActors = 4096;
constexpr float kScanRadius = 64.0f;

struct Vec2 { float x, y; };
struct Vec3 { float x, y, z; };
using GetRuntimeActorList = std::vector<void*>(*)(void*);
using ActorIsPlayer = bool(*)(void*);
using ActorGetNameTag = std::string(*)(void*);

std::mutex gMutex;
void* gHandle = nullptr;
const ApiV1* gApi = nullptr;
std::uint64_t gTickSubscription = 0, gActionSubscription = 0, gAttackSubscription = 0;
std::atomic<bool> gActorManagerListSignatureAvailable{false};
std::atomic<bool> gActorRuntimeCapabilitiesReady{false};
std::atomic<bool> gActorEnumerationRequested{true};
std::atomic<bool> gActorEnumerationEnabled{false};
std::atomic<GetRuntimeActorList> gGetRuntimeActorList{nullptr};
std::atomic<ActorIsPlayer> gActorIsPlayer{nullptr};
std::atomic<ActorGetNameTag> gActorGetNameTag{nullptr};
std::atomic<std::uint64_t> gActorEnumerationCalls{0}, gLastActorCount{0}, gActorEnumerationFailures{0};
std::atomic<std::uint64_t> gTicks{0}, gActions{0}, gAttacks{0};
std::atomic<bool> gWorldSessionSeen{false};
std::atomic<std::uint64_t> gBreaks{0}, gPlaces{0}, gInteracts{0}, gUses{0};

// These prefixes mirror the public BedrockTools event declarations. Only the
// leading fields are read. The Player pointer is used synchronously on the
// game thread and is never retained beyond this callback.
struct LocalPlayerTickPrefix { void* player; };
struct GameModeActionPrefix { std::uint8_t modeKind; std::uint8_t action; };
static_assert(offsetof(LocalPlayerTickPrefix, player) == 0);
static_assert(offsetof(GameModeActionPrefix, action) == 1);

bool readActorTransform(void* actor, Vec3& position, Vec2& rotation) noexcept {
    if (!actor) return false;
    const auto base = reinterpret_cast<std::uintptr_t>(actor);
    auto* state = *reinterpret_cast<void**>(base + kActorStateVectorOffset);
    auto* turn = *reinterpret_cast<void**>(base + kActorRotationOffset);
    if (!state || !turn) return false;
    std::memcpy(&position, state, sizeof(position));
    std::memcpy(&rotation, turn, sizeof(rotation));
    return std::isfinite(position.x) && std::isfinite(position.y) &&
           std::isfinite(position.z) && std::isfinite(rotation.x) &&
           std::isfinite(rotation.y);
}

struct Candidate {
    std::uint64_t runtimeKey{};
    float x{}, y{}, z{}, yaw{}, pitch{}, distanceSquared{};
    bool isPlayer{};
    std::string displayName;
};

void publishNearbyActors(void* player) {
    auto getActors = gGetRuntimeActorList.load(std::memory_order_acquire);
    auto isPlayerFn = gActorIsPlayer.load(std::memory_order_acquire);
    auto getNameTagFn = gActorGetNameTag.load(std::memory_order_acquire);
    if (!player || !getActors || !isPlayerFn || !getNameTagFn) {
        entities::resetRecognition();
        gActorEnumerationFailures.fetch_add(1, std::memory_order_relaxed);
        gLastActorCount.store(0, std::memory_order_relaxed);
        return;
    }

    Vec3 playerPosition{};
    Vec2 playerRotation{};
    if (!readActorTransform(player, playerPosition, playerRotation)) {
        entities::resetRecognition();
        gActorEnumerationFailures.fetch_add(1, std::memory_order_relaxed);
        gLastActorCount.store(0, std::memory_order_relaxed);
        return;
    }

    auto* level = *reinterpret_cast<void**>(
        reinterpret_cast<std::uintptr_t>(player) + kPlayerLevelOffset);
    auto* actorManager = level
        ? *reinterpret_cast<void**>(reinterpret_cast<std::uintptr_t>(level) + kLevelActorManagerOffset)
        : nullptr;
    if (!actorManager) {
        entities::resetRecognition();
        gActorEnumerationFailures.fetch_add(1, std::memory_order_relaxed);
        gLastActorCount.store(0, std::memory_order_relaxed);
        return;
    }

    // ActorManagerList is an internal BedrockTools iterator, not a stable public
    // API. It is invoked only on LocalPlayerTick, and its pointers are consumed
    // immediately; no Actor pointer or vector storage escapes this function.
    const auto actors = getActors(actorManager);
    if (actors.size() > kMaxRuntimeActors) {
        entities::resetRecognition();
        gActorEnumerationFailures.fetch_add(1, std::memory_order_relaxed);
        gLastActorCount.store(0, std::memory_order_relaxed);
        return;
    }

    std::vector<Candidate> candidates;
    candidates.reserve(std::min(actors.size(), kMaxRuntimeActors));
    for (void* actor : actors) {
        if (!actor || actor == player) continue;
        Vec3 position{};
        Vec2 rotation{};
        if (!readActorTransform(actor, position, rotation)) continue;

        const float dx = position.x - playerPosition.x;
        const float dy = position.y - playerPosition.y;
        const float dz = position.z - playerPosition.z;
        const float distanceSquared = dx * dx + dy * dy + dz * dz;
        if (!std::isfinite(distanceSquared) || distanceSquared > kScanRadius * kScanRadius) continue;

        const bool playerActor = isPlayerFn(actor);
        std::string name = getNameTagFn(actor);
        if (name.empty()) name = playerActor ? "Player" : "Unnamed mob";
        if (name.size() >= 64) name.resize(63);

        Candidate candidate{};
        candidate.runtimeKey = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(actor));
        candidate.x = position.x; candidate.y = position.y; candidate.z = position.z;
        candidate.yaw = rotation.y; candidate.pitch = rotation.x;
        candidate.distanceSquared = distanceSquared;
        candidate.isPlayer = playerActor;
        candidate.displayName = std::move(name);
        candidates.push_back(std::move(candidate));
    }

    std::sort(candidates.begin(), candidates.end(),
        [](const Candidate& a, const Candidate& b) {
            return a.distanceSquared < b.distanceSquared;
        });

    const auto count = std::min(candidates.size(), entities::kMaxEntitiesPerFrame);
    std::array<std::string, entities::kMaxEntitiesPerFrame> names{};
    std::array<entities::EntitySample, entities::kMaxEntitiesPerFrame> samples{};
    for (std::size_t i = 0; i < count; ++i) {
        const auto& c = candidates[i];
        names[i] = c.displayName;
        samples[i] = entities::EntitySample{
            c.runtimeKey,
            c.isPlayer ? std::string_view("minecraft:player") : std::string_view("minecraft:entity"),
            names[i],
            c.x, c.y, c.z, c.yaw, c.pitch,
            c.isPlayer, true
        };
    }

    if (!entities::ingestVerifiedFrame(
            std::span<const entities::EntitySample>(samples.data(), count),
            playerPosition.x, playerPosition.y, playerPosition.z, kScanRadius)) {
        entities::resetRecognition();
        gActorEnumerationFailures.fetch_add(1, std::memory_order_relaxed);
        gLastActorCount.store(0, std::memory_order_relaxed);
        return;
    }
    gLastActorCount.store(count, std::memory_order_relaxed);
    gActorEnumerationCalls.fetch_add(1, std::memory_order_relaxed);
}

void onEvent(EventType type, void* payload, void*) {
    if (!payload) return;
    if (type == kLocalPlayerTick) {
        const auto* event = static_cast<const LocalPlayerTickPrefix*>(payload);
        if (event->player) {
            gTicks.fetch_add(1, std::memory_order_relaxed);
            gWorldSessionSeen.store(true, std::memory_order_release);
            if (gActorEnumerationEnabled.load(std::memory_order_acquire)) {
                try {
                    publishNearbyActors(event->player);
                } catch (...) {
                    entities::resetRecognition();
                    gActorEnumerationFailures.fetch_add(1, std::memory_order_relaxed);
                    gLastActorCount.store(0, std::memory_order_relaxed);
                }
            }
        }
    } else if (type == kGameModeAction) {
        const auto* event = static_cast<const GameModeActionPrefix*>(payload);
        gActions.fetch_add(1, std::memory_order_relaxed);
        // Public GameModeAction enum values: destroy start/stop=0/1,
        // build/place=2, use item=3, use-as-attack=4, use-on=5, interact=6.
        switch (event->action) {
            case 0: case 1: gBreaks.fetch_add(1, std::memory_order_relaxed); break;
            case 2: gPlaces.fetch_add(1, std::memory_order_relaxed); break;
            case 3: case 4: case 5: gUses.fetch_add(1, std::memory_order_relaxed); break;
            case 6: gInteracts.fetch_add(1, std::memory_order_relaxed); break;
            default: break;
        }
    } else if (type == kAttack) {
        gAttacks.fetch_add(1, std::memory_order_relaxed);
    }
}

void releaseLocked() noexcept {
    if (gApi) {
        if (gTickSubscription) gApi->unsubscribe(gTickSubscription);
        if (gActionSubscription) gApi->unsubscribe(gActionSubscription);
        if (gAttackSubscription) gApi->unsubscribe(gAttackSubscription);
    }
    gTickSubscription = gActionSubscription = gAttackSubscription = 0;
    gActorEnumerationEnabled.store(false, std::memory_order_release);
    gActorRuntimeCapabilitiesReady.store(false, std::memory_order_release);
    gGetRuntimeActorList.store(nullptr, std::memory_order_release);
    gActorIsPlayer.store(nullptr, std::memory_order_release);
    gActorGetNameTag.store(nullptr, std::memory_order_release);
    gActorManagerListSignatureAvailable.store(false, std::memory_order_relaxed);
    gApi = nullptr;
    if (gHandle) dlclose(gHandle);
    gHandle = nullptr;
}
}

bool start() noexcept {
    std::lock_guard lock(gMutex);
    if (gApi && (gTickSubscription || gActionSubscription || gAttackSubscription)) return true;
    releaseLocked();
    gHandle = dlopen("libBedrockTools.so", RTLD_NOW | RTLD_NOLOAD);
    if (!gHandle) return false;
    auto getter = reinterpret_cast<GetApi>(dlsym(gHandle, "BedrockTools_GetApi"));
    gApi = getter ? getter(kAbiVersion) : nullptr;
    if (!gApi || gApi->abiVersion != kAbiVersion ||
        gApi->structSize < sizeof(ApiV1) || !gApi->subscribe ||
        !gApi->unsubscribe || !gApi->resolveSignature) {
        releaseLocked();
        return false;
    }

    const auto actorListAddress = gApi->resolveSignature(kActorManagerListSignatureId);
    const auto actorIsPlayerAddress = gApi->resolveSignature(kActorIsPlayerSignatureId);
    const auto actorGetNameTagAddress = gApi->resolveSignature(kActorGetNameTagSignatureId);
    gActorManagerListSignatureAvailable.store(actorListAddress != 0, std::memory_order_relaxed);
    gGetRuntimeActorList.store(reinterpret_cast<GetRuntimeActorList>(actorListAddress), std::memory_order_release);
    gActorIsPlayer.store(reinterpret_cast<ActorIsPlayer>(actorIsPlayerAddress), std::memory_order_release);
    gActorGetNameTag.store(reinterpret_cast<ActorGetNameTag>(actorGetNameTagAddress), std::memory_order_release);
    const bool ready = actorListAddress && actorIsPlayerAddress && actorGetNameTagAddress;
    gActorRuntimeCapabilitiesReady.store(ready, std::memory_order_release);
    gActorEnumerationEnabled.store(
        ready && gActorEnumerationRequested.load(std::memory_order_acquire),
        std::memory_order_release);

    gTickSubscription = gApi->subscribe(kLocalPlayerTick, kNormalPriority, onEvent, nullptr);
    gActionSubscription = gApi->subscribe(kGameModeAction, kNormalPriority, onEvent, nullptr);
    gAttackSubscription = gApi->subscribe(kAttack, kNormalPriority, onEvent, nullptr);
    if (!gTickSubscription || !gActionSubscription || !gAttackSubscription) {
        releaseLocked();
        return false;
    }
    return true;
}

void setActorEnumerationEnabled(bool enabled) noexcept {
    gActorEnumerationRequested.store(enabled, std::memory_order_release);
    const bool canEnable = enabled &&
        gActorRuntimeCapabilitiesReady.load(std::memory_order_acquire) &&
        gGetRuntimeActorList.load(std::memory_order_acquire) != nullptr &&
        gActorIsPlayer.load(std::memory_order_acquire) != nullptr &&
        gActorGetNameTag.load(std::memory_order_acquire) != nullptr;
    gActorEnumerationEnabled.store(canEnable, std::memory_order_release);
    if (!canEnable) {
        entities::resetRecognition();
        gLastActorCount.store(0, std::memory_order_relaxed);
    }
}

void stop() noexcept {
    std::lock_guard lock(gMutex);
    releaseLocked();
    entities::resetRecognition();
}

Snapshot snapshot() noexcept {
    std::lock_guard lock(gMutex);
    return {gTicks.load(std::memory_order_relaxed), gActions.load(std::memory_order_relaxed),
            gAttacks.load(std::memory_order_relaxed), gBreaks.load(std::memory_order_relaxed),
            gPlaces.load(std::memory_order_relaxed), gInteracts.load(std::memory_order_relaxed),
            gUses.load(std::memory_order_relaxed), gApi != nullptr,
            gTickSubscription != 0 && gActionSubscription != 0 && gAttackSubscription != 0,
            gActorManagerListSignatureAvailable.load(std::memory_order_relaxed),
            gActorEnumerationEnabled.load(std::memory_order_relaxed),
            gActorEnumerationCalls.load(std::memory_order_relaxed),
            gLastActorCount.load(std::memory_order_relaxed),
            gActorEnumerationFailures.load(std::memory_order_relaxed),
            gWorldSessionSeen.load(std::memory_order_acquire)};
}
}
