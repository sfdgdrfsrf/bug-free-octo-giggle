# Bedrock Runtime Integration

## Target and safety rule

The requested target is Minecraft Bedrock Android 26.62, arm64-v8a, loaded by LeviLaunchroid. The target is not runtime-verified. A successful Android compile only proves that the native libraries link; it does not prove that a Bedrock object layout, signature, vtable, event payload, or callback lifetime is valid in 26.62.

All gameplay integrations must fail closed unless the runtime adapter reports a verified game build and the required capability. No guessed offsets, raw actor-manager traversal, retained game pointers, or world writes are allowed.

## Verified public integration surface inspected

The public BedrockTools source currently exposes an ABI-v1 table with:
- ABI version and structure size;
- signature resolver;
- client-instance getter;
- event subscribe/unsubscribe functions.

Its documented event declarations include:
- LocalPlayerTick (payload begins with a Player pointer);
- GameModeAction (mode/action enums and a game-mode pointer);
- Attack;
- several UI/input/frame events.

The current MobTalk bridge subscribes to LocalPlayerTick, GameModeAction, and Attack and counts action categories. It only reads the leading fields documented by those event declarations. It does not retain or dereference the pointers.

The public BedrockTools SDK also contains wrappers for ClientInstance, Player/Actor, Level, movement input, inventory offsets, and rendering offsets. These wrappers use version-specific signatures and direct object offsets. The Level wrapper exposes actorManager only as an opaque pointer; the inspected public SDK does not provide a supported actor-enumeration API through the external ABI. Inventory and rendering offsets are declarations, not stable cross-version APIs.

Reference source inspected (not vendored):
- https://github.com/QYCottage/BedrockTools
- https://github.com/QYCottage/BedrockTools/blob/main/include/bedrocktools/Api.hpp
- https://github.com/QYCottage/BedrockTools/blob/main/include/bedrocktools/events/LocalPlayerTickEvent.hpp
- https://github.com/QYCottage/BedrockTools/blob/main/include/bedrocktools/events/GameModeActionEvent.hpp
- https://github.com/QYCottage/BedrockTools/blob/main/include/bedrocktools/sdk/world/Actor.hpp
- https://github.com/QYCottage/BedrockTools/blob/main/include/bedrocktools/sdk/world/Level.hpp
- https://github.com/QYCottage/BedrockTools/blob/main/include/bedrocktools/sdk/client/ClientInstance.hpp

## Subsystem integration matrix

| System | Public surface found | Current safe status | Missing proof/API |
|---|---|---|---|
| Entity discovery and identity | LocalPlayerTick carries local Player pointer; Actor wrapper has position/rotation/name accessors | Event notification only | Stable enumeration of non-player actors; 26.62 object-layout/signature validation; identity lifetime rules |
| Inventory | BedrockTools has inventory offset constants and an internal InventoryAccess implementation | Not connected | Supported external inventory API, slot/item lifetime rules, transaction authority, build validation |
| Movement/navigation | Movement input component layout and player tick event | Planner core only | Supported actor movement/command API, collision/world queries, game-thread contract |
| Rendering | Public render offsets and renderer wrappers | No custom entity rendering | Stable render callback/context, resource lifetime, 26.62 offsets and renderer validation |
| Chat | MobTalk local HUD/input and transcript persistence | Local preview only | Incoming chat event, safe outbound chat API, entity chat bubble/render hook, multiplayer semantics |
| World/block interaction | GameModeAction event observes player actions | Telemetry only | Read/write block API, inventory transaction API, permission/authority semantics |

## Required staged implementation

1. **Runtime identity gate:** obtain a runtime-provided Minecraft build identifier and ABI/capability version. Refuse gameplay hooks when the build is not explicitly allowlisted. BedrockTools ABI v1 as inspected does not expose a game-build identifier, so its presence alone is insufficient.
2. **Read-only local-player adapter:** validate the 26.62 signature and callback thread/lifetime rules, then copy only bounded scalar data into pointer-free snapshots during the callback. Never store Actor/Player pointers.
3. **Entity enumeration:** require an official iterator or a separately verified, version-pinned adapter. Do not walk the opaque actorManager pointer.
4. **Inventory and world queries:** require typed APIs with explicit ownership, bounds, and game-thread rules. Read-only inventory inspection precedes any transaction or mutation.
5. **Movement and actions:** add one action at a time behind opt-in toggles, with range/collision checks, cancellation, and a no-op mode.
6. **Rendering and chat:** validate render-thread callback/context and chat send/receive APIs independently; neither may be inferred from HUD drawing or action events.
7. **Device validation:** test on a backed-up disposable world with the exact Minecraft 26.62 Android binary and installed LeviLaunchroid/BedrockTools versions. Run read-only first, then each mutation capability separately.

## Current implementation truth

- LeviMania's six feature engines are tested core logic, not connected to live Bedrock actors.
- MobTalk has a local preview console, persistent preview data, and optional player-action telemetry. It is not yet a creature-chat controller.
- All experimental gameplay toggles remain off by default.
- The existing event bridge is useful telemetry, not proof of entity, inventory, movement, rendering, or chat compatibility.

Do not mark a subsystem as integrated until its API contract, exact runtime build, pointer lifetime, thread affinity, failure behavior, tests, and in-game validation are documented.
