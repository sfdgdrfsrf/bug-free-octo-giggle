#include "mobtalk/runtime_bridge.hpp"
#include <dlfcn.h>
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
std::mutex gMutex;
void* gHandle = nullptr;
const ApiV1* gApi = nullptr;
std::uint64_t gTickSubscription = 0, gActionSubscription = 0, gAttackSubscription = 0;
std::atomic<std::uint64_t> gTicks{0}, gActions{0}, gAttacks{0};
void onEvent(EventType type, void*, void*) {
    // Treat payloads as opaque. Their internal layout belongs to the
    // version-specific BedrockTools adapter; never dereference or retain them.
    if (type == kLocalPlayerTick) {
        gTicks.fetch_add(1, std::memory_order_relaxed);
    } else if (type == kGameModeAction) {
        gActions.fetch_add(1, std::memory_order_relaxed);
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
        gApi->structSize < sizeof(ApiV1) || !gApi->subscribe || !gApi->unsubscribe) {
        releaseLocked();
        return false;
    }
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
            gAttacks.load(std::memory_order_relaxed), gApi != nullptr,
            gTickSubscription != 0 || gActionSubscription != 0 || gAttackSubscription != 0};
}
}
