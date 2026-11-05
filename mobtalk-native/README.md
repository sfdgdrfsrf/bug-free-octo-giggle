# MobTalk Native

A separate LeviLaunchroid `.levipack` target for an independently authored native creature-conversation layer inspired by the documented feature set of CreatureChat.

## Target compatibility

The observed target is **Minecraft Bedrock Android 26.60 (arm64-v8a)** with **BedrockTools v1.5.10**, as displayed in the user's launcher screenshot. The exact game binary/build ID and runtime signatures are not verified. The generic Android build does not prove 26.60 compatibility. See [COMPATIBILITY.md](COMPATIBILITY.md) for the validation checklist and fail-closed rules.

## Implemented

- Per-creature identity/personality, bounded conversation history, friendship, and allow-listed behavior-marker parsing.
- Bounded conversation-context construction and player-intent classification.
- Bounded binary memory storage with version/size checks and failure-safe loading.
- Native PreLoader HUD console, on-screen Talk button, Android keyboard input callback, and a read-only Nearby Entities list panel. The panel automatically marks a world session active after the first local-player tick.
- Local preview transcript save/load under the mod data directory.
- Optional runtime event bridge to BedrockTools API ABI v1: local-player tick, game-mode action, and attack subscriptions, with block-break/place, interaction, and item-use counters. It safely fails closed if BedrockTools is not installed or its API is incompatible.
- Read-only availability check for BedrockTools v1.5.10's `ActorManagerList` signature (SignatureId ordinal 12). The actor-count probe is **hard-locked OFF in code** and cannot be enabled by config; it is not invoked.
- Host tests for conversation, persistence, runtime bridge lifecycle, and the pointer-free entity snapshot store.
- Pointer-free entity snapshot core: bounded to 128 entities per frame, copies type/name and position/rotation into fixed-size owned buffers, rejects non-finite coordinates and duplicate transient keys, and synchronizes publication/readback. This is a tested data layer only; no Bedrock actor adapter feeds it yet.


## Runtime bridge requirements

For the optional event bridge to connect, install a compatible BedrockTools build that exports `BedrockTools_GetApi` from `libBedrockTools.so` (API ABI version 1). MobTalk uses the documented public C ABI shape and subscribes to the published event IDs. It does not copy BedrockTools source or its internal SDK headers. BedrockTools' own version-specific signatures/offsets remain owned by that runtime.

The current bridge observes player-tick, game-mode-action, and attack event notifications. The `actor_enumeration_probe` is hard-locked OFF in code, so a config toggle cannot invoke it. It uses a version-specific internal BedrockTools function and Level offsets; do not unlock it until the exact Minecraft 26.60 binary, offsets, and C++ ABI are verified. It is not a supported entity API and is not yet entity identity or behavior integration.

## Important live-runtime boundary

This is **not yet a complete CreatureChat port**. The HUD and input console are connected to PreLoader; the optional BedrockTools event bridge can observe the three event types above. Messages submitted in the console are still saved to a local preview transcript and are **not sent to a nearby creature**.

Still not connected: live nearby-entity enumeration and identity (the list panel is a viewer for the recognition registry, not an entity source), receiving/sending player chat events, secure AI HTTP requests, entity navigation/targeting, inventory transactions, particles, entity-attached chat bubbles, a dedicated native chat screen, multiplayer synchronization, and automatic per-world creature memory lifecycle. No API key is accepted or stored until a secure request adapter exists. Entity-affecting and network features remain disabled.

The bridge deliberately avoids guessed function offsets, vtable slots, and undocumented entity layouts. BedrockTools' own public SDK includes version-specific wrappers and signatures, but its README and LICENSE file contain inconsistent license statements; MobTalk therefore uses only the documented runtime ABI and does not bundle its source or headers. The original CreatureChat project is a Java Fabric mod; this package does not load its Java `.jar`.

## Package

The root workflow builds `mobtalk-native-arm64.levipack` separately from `levimania-mobile-arm64.levipack`.

## Licensing

The referenced CreatureChat repository identifies source code as GPL-3.0-or-later and non-code assets as CC-BY-NC-SA-4.0, with separate trademark restrictions. This native implementation does not bundle its code or protected assets. See `LICENSE-NOTICE.md`; no endorsement is implied.

- Entity recognition pipeline: verified adapters can submit pointer-free frames, maintain a bounded nearby-creature list, assign session-only target IDs, and look up the selected creature. The new Nearby Entities panel displays up to 10 entries from this registry. It automatically detects the local-player tick/world-session signal, but reports an empty/waiting state because no verified Bedrock actor adapter feeds the pipeline yet; opening the panel does not falsely claim that game entities were scanned.
