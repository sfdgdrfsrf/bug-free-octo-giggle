# MobTalk Native

A separate LeviLaunchroid `.levipack` target for an independently authored native creature-conversation layer inspired by the documented feature set of CreatureChat.

## Target compatibility

Target: **Minecraft Bedrock Android 26.60 (arm64-v8a)** with **BedrockTools v1.5.10**. BedrockTools' v1.5.10 source includes its 26.60 support update (commit [3de9a7d](https://github.com/QYCottage/BedrockTools/commit/3de9a7d641c8222a9bf536d828449a27b58cd20a)) and the matching actor/level offsets. This is source-level compatibility evidence, not a substitute for testing the exact installed game binary. See [RUNTIME_INTEGRATION.md](../RUNTIME_INTEGRATION.md).

## Implemented

- Per-creature identity/personality, bounded conversation history, friendship, and allow-listed behavior-marker parsing.
- Bounded conversation-context construction and player-intent classification.
- Bounded binary memory storage with version/size checks and failure-safe loading.
- Native PreLoader HUD console, on-screen Talk button, Android keyboard input callback, and Nearby Entities scanner.
- Read-only BedrockTools API ABI-v1 bridge for local-player tick, game-mode action, and attack events.
- **Live read-only entity enumeration is now connected.** On each local-player tick, MobTalk resolves the BedrockTools v1.5.10 actor-list, actor-is-player, and actor-name-tag signatures; reads the 26.60 actor/level transform fields; copies nearby actor data into pointer-free snapshots; and feeds the Nearby Entities registry. The scan radius is 64 blocks and the snapshot is capped at 128 entities per frame; the visible tracker keeps the nearest 32.
- The scanner is enabled by default because the user explicitly requested unlocking it. It remains behind a runtime capability gate: all three required signatures and the BedrockTools ABI must be present. The `actor_enumeration_probe` setting can disable it.
- The scanner identifies player actors separately. For non-player actors, the current adapter displays a custom name tag when present or “Unnamed mob”; BedrockTools v1.5.10 does not expose a stable public entity-type identifier through its external ABI, so the displayed type remains generic.
- No Actor/Player pointers, actor-list vector storage, or runtime string references are retained after the game-thread callback.
- Host tests cover conversation, persistence, runtime bridge lifecycle, pointer-free snapshots, tracking, and recognition.

## Runtime bridge and safety

The actor-list call is an internal BedrockTools implementation detail, not a stable public entity-iterator API. BedrockTools v1.5.10 itself uses the same `ActorManagerList` signature and `std::vector<void*>(*)(void*)` call pattern in its Tablist and Debug Menu modules. MobTalk additionally requires the matching `ActorIsPlayer` and `ActorGetNameTag` signatures before enabling the scanner.

The target offsets are the values in BedrockTools v1.5.10's 26.60 support source: Player Level `0x1F0`, Level actor manager `0x478`, Actor state-vector component `0x228`, and Actor rotation component `0x238`. The adapter runs synchronously from LocalPlayerTick, bounds the actor list to 4096 entries, filters to 64 blocks, sorts by distance, and copies no more than 128 records. The visible tracker keeps the nearest 32.

**Important:** successful compilation and signature resolution do not prove that the user's installed binary is identical to the inspected 26.60 build. This adapter uses version-specific internal offsets and C++ function signatures; a mismatch can still crash Minecraft. Test first in a disposable world with a backup. Turn off `actor_enumeration_probe` immediately if the game crashes or the list behaves incorrectly. No in-game validation has yet been claimed.

## Remaining live systems

This is not yet a complete CreatureChat port. The HUD and input console are connected, and nearby entity snapshots now feed the scanner. Messages submitted in the console are still saved to a local preview transcript and are **not sent to a nearby creature**.

Still not connected: receiving/sending player chat events, secure AI HTTP requests, entity navigation/targeting, inventory transactions, particles, entity-attached chat bubbles, multiplayer synchronization, and automatic per-world creature memory lifecycle. No API key is accepted or stored until a secure request adapter exists. Entity-affecting and network features remain disabled.

## Package

The root workflow builds `mobtalk-native-arm64.levipack` separately from `levimania-mobile-arm64.levipack`.

## Licensing

The referenced CreatureChat repository identifies source code as GPL-3.0-or-later and non-code assets as CC-BY-NC-SA-4.0, with separate trademark restrictions. This native implementation does not bundle its code or protected assets. See `LICENSE-NOTICE.md`; no endorsement is implied.
