# Bedrock Runtime Integration

## Target and safety rule

Target: Minecraft Bedrock Android **26.60**, arm64-v8a, loaded by LeviLaunchroid, with **BedrockTools v1.5.10**. BedrockTools v1.5.10 includes the 26.60 support update from commit [3de9a7d](https://github.com/QYCottage/BedrockTools/commit/3de9a7d641c8222a9bf536d828449a27b58cd20a). Its tagged source provides the offsets and signature patterns used by the MobTalk scanner. This does not independently fingerprint the user's installed `libminecraftpe.so` or replace device testing.

Gameplay integrations remain capability-gated. MobTalk does not retain raw actor pointers, actor-list vector storage, or component pointers after the LocalPlayerTick callback. The scanner is read-only.

## Verified BedrockTools source surface

The inspected v1.5.10 public ABI provides:
- ABI version and structure size;
- signature resolver;
- client-instance getter;
- event subscribe/unsubscribe functions.

The documented event declarations include LocalPlayerTick, GameModeAction, Attack, and several UI/input/frame events. MobTalk subscribes to LocalPlayerTick, GameModeAction, and Attack. The LocalPlayerTick event is the game-thread callback used for the scanner.

BedrockTools v1.5.10's own Tablist and Debug Menu modules use the internal `ActorManagerList` signature as `std::vector<void*>(*)(void* actorManager)`. Tablist also uses `ActorIsPlayer` and `ActorGetNameTag`, whose function types are `bool(*)(void*)` and `std::string(*)(void*)`. These are internal runtime functions exposed indirectly through the generic signature resolver; they are not a stable external entity-iterator contract.

The v1.5.10 source includes the 26.60 support update. The matching offsets used by the adapter are:
- Player Level: `0x1F0`
- Level actor manager: `0x478`
- Actor state-vector component: `0x228`
- Actor rotation component: `0x238`

Signature ordinals in the v1.5.10 enum:
- ActorManagerList: 12
- ActorIsPlayer: 66
- ActorGetNameTag: 69

## MobTalk scanner implementation

The live scanner is now connected to the recognition pipeline:

1. At enable time, MobTalk obtains BedrockTools API ABI v1 and resolves all three required signatures. The scanner activates only if the API and all signatures are present.
2. On LocalPlayerTick, it reads the player's Level and actor manager using the 26.60 offsets, then calls ActorManagerList synchronously on the game thread.
3. It rejects actor lists larger than 4096 entries, skips null/self actors, reads each actor's state-vector and rotation components, rejects non-finite transforms, and filters to a 64-block radius.
4. It calls ActorIsPlayer and ActorGetNameTag while each actor pointer is valid in the callback. Custom name tags are copied; unnamed non-player entities are labelled “Unnamed mob”. Player entities are identified separately.
5. It sorts by distance, copies up to 128 pointer-free EntitySample records into the recognition pipeline, and the tracker keeps the nearest 32 session entities.
6. No runtime pointer or std::string reference survives the callback. Disabling the scanner clears the recognition registry.

The `actor_enumeration_probe` configuration switch is now unlocked and defaults to enabled, as explicitly requested. It can be turned off from the MobTalk module settings. The scanner will remain off automatically if BedrockTools is absent, ABI-incompatible, or any required signature fails to resolve.

**Entity-type limitation:** BedrockTools' inspected external ABI does not provide a stable entity identifier/name API for every mob. The scanner currently identifies players and lists non-player actors as generic entities, using custom name tags where available. Exact identifiers such as `minecraft:pig` or `minecraft:zombie` are not yet available from this adapter.

## Runtime risk and validation

The scanner relies on version-specific BedrockTools internals and direct object offsets. A signature match is useful evidence, but it is not a cryptographic build fingerprint and does not guarantee that the user's installed binary has the expected object layout or C++ ABI. A bad match can still cause an access violation; C++ exception handling cannot catch invalid memory access.

The adapter is read-only and bounded, but it has **not yet been verified in a live Minecraft 26.60 session**. Test first in a backed-up disposable world. If Minecraft crashes, disable `actor_enumeration_probe` before launching again and report the crash point/log.

## Remaining integration matrix

| System | Current status | Remaining work |
|---|---|---|
| Entity discovery | Read-only actor enumeration feeds pointer-free nearby snapshots | Device test; exact mob type identifiers; world-leave lifecycle |
| Inventory | Not connected | Supported external inventory API, transaction semantics, build validation |
| Movement/navigation | Planner core only | Supported actor movement API, collision/world queries, cancellation |
| Rendering | No custom entity rendering | Stable render callback/context and resource lifetime |
| Chat | Local preview console only | Incoming/outgoing chat APIs and entity chat bubble |
| World/block interaction | Player action telemetry only | Typed world/block APIs and authoritative transaction rules |
| AI network | Disabled | Secure request adapter, consent, key storage, bounded responses |

## Current implementation truth

- The scanner now attempts automatic nearby actor enumeration when the 26.60 BedrockTools capability signatures resolve.
- The scanner copies data into owned snapshots; it does not retain Bedrock actor pointers.
- Non-player entity type IDs remain generic; exact mob identifiers are not yet available.
- Console messages remain local preview only and are not delivered to mobs.
- Inventory, movement, rendering, chat, AI networking, and gameplay behavior are not integrated.
- Compilation and CI tests do not substitute for in-game runtime validation.
