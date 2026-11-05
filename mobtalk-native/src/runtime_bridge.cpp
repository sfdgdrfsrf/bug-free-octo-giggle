#include "mobtalk/runtime_bridge.hpp"
#include <dlfcn.h>
#include <cstddef>
#include <mutex>
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
// BedrockTools v1.5.10 SignatureId::ActorManagerList is ordinal 12.
// Probe only: the function is used internally by BedrockTools but is not a documented external iterator.
constexpr std::uint16_t kActorManagerListSignatureId = 12;
std::mutex gMutex;
void* gHandle = nullptr;
const ApiV1* gApi = nullptr;
std::uint64_t gTickSubscription = 0, gActionSubscription = 0, gAttackSubscription = 0;
std::atomic<bool> gActorManagerListSignatureAvailable{false};
std::atomic<std::uint64_t> gTicks{0}, gActions{0}, gAttacks{0};
std::atomic<std::uint64_t> gBreaks{0}, gPlaces{0}, gInteracts{0}, gUses{0};

// These prefixes mirror the public BedrockTools event declarations. Only the
// leading fields are read; game pointers and the rest of the payload are never
// retained or dereferenced.
struct LocalPlayerTickPrefix { void* player; };
struct GameModeActionPrefix { std::uint8_t modeKind; std::uint8_t action; };
static_assert(offsetof(LocalPlayerTickPrefix, player) == 0);
static_assert(offsetof(GameModeActionPrefix, action) == 1);

void onEvent(EventType type, void* payload, void*) {
    if (!payload) return;
    if (type == kLocalPlayerTick) {
        const auto* event = static_cast<const LocalPlayerTickPrefix*>(payload);
        if (event->player) gTicks.fetch_add(1, std::memory_order_relaxed);
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
        gApi->structSize < sizeof(ApiV1) || !gApi->subscribe || !gApi->unsubscribe || !gApi->resolveSignature) {
        releaseLocked();
        return false;
    }
    // Read-only capability probe. Never invoke the resolved function here.
    gActorManagerListSignatureAvailable.store(
        gApi->resolveSignature(kActorManagerListSignatureId) != 0, std::memory_order_relaxed);
    gTickSubscription = gApi->subscribe(kLocalPlayerTick, kNormalPriority, onEvent, nullptr);
    gActionSubscription = gApi->subscribe(kGameModeAction, kNormalPriority, onEvent, nullptr);
    gAttackSubscription = gApi->subscribe(kAttack, kNormalPriority, onEvent, nullptr);
    // The bridge is considered connected only when every advertised event
    // subscription succeeds. Partial registration is rolled back fail-closed.
    if (!gTickSubscription || !gActionSubscription || !gAttackSubscription) {
        releaseLocked();
        return false;
    }
    return true;
}
void stop() noexcept {
    std::lock_guard lock(gMutex);
    releaseLocked();
}
Snapshot snapshot() noexcept {
    std::lock_guard lock(gMutex);
    return {gTicks.load(std::memory_order_relaxed), gActions.load(std::memory_order_relaxed),
            gAttacks.load(std::memory_order_relaxed), gBreaks.load(std::memory_order_relaxed),
            gPlaces.load(std::memory_order_relaxed), gInteracts.load(std::memory_order_relaxed),
            gUses.load(std::memory_order_relaxed), gApi != nullptr,
            gTickSubscription != 0 && gActionSubscription != 0 && gAttackSubscription != 0,
            gActorManagerListSignatureAvailable.load(std::memory_order_relaxed)};
}
}
