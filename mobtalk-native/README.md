# MobTalk Native

A separate LeviLaunchroid `.levipack` target for an independently authored native creature-conversation layer inspired by the documented feature set of CreatureChat.

## Target compatibility

The requested target is **Minecraft Bedrock Android 26.62 (arm64-v8a)**. This is not yet a verified live-runtime target: LeviLaunchroid v1.6.0 release notes document 26.60 support, and BedrockTools release metadata/source warn that signatures and offsets are version-specific. The generic Android build does not prove 26.62 compatibility. See [COMPATIBILITY.md](COMPATIBILITY.md) for the validation checklist and fail-closed rules.

## Implemented

- Per-creature identity/personality, bounded conversation history, friendship, and allow-listed behavior-marker parsing.
- Bounded conversation-context construction and player-intent classification.
- Bounded binary memory storage with version/size checks and failure-safe loading.
- Native PreLoader HUD console, on-screen Talk button, and Android keyboard input callback.
- Local preview transcript save/load under the mod data directory.
- Optional runtime event bridge to BedrockTools API ABI v1: local-player tick, game-mode action, and attack subscriptions, with block-break/place, interaction, and item-use counters. It safely fails closed if BedrockTools is not installed or its API is incompatible.
- Read-only probe for BedrockTools v1.5.10's `ActorManagerList` signature (SignatureId ordinal 12), plus an **OFF-by-default experimental actor-count probe**. If manually enabled, it invokes the same internal function pattern used by BedrockTools Tablist during LocalPlayerTick and counts non-null entries only (maximum 4,096). It never dereferences or retains actor pointers.
- Host tests for conversation, persistence, and runtime bridge lifecycle.

## Runtime bridge requirements

For the optional event bridge to connect, install a compatible BedrockTools build that exports `BedrockTools_GetApi` from `libBedrockTools.so` (API ABI version 1). MobTalk uses the documented public C ABI shape and subscribes to the published event IDs. It does not copy BedrockTools source or its internal SDK headers. BedrockTools' own version-specific signatures/offsets remain owned by that runtime.

The current bridge observes player-tick, game-mode-action, and attack event notifications. The optional `actor_enumeration_probe` setting is OFF by default. Enabling it invokes a version-specific internal BedrockTools function and reads version-specific Level offsets; it may crash on an unsupported Minecraft build. Only use it on a backed-up disposable world after confirming the exact game/runtime combination. It counts actor pointers but does not dereference or retain them. This is not yet entity identity or behavior integration.

## Important live-runtime boundary

This is **not yet a complete CreatureChat port**. The HUD and input console are connected to PreLoader; the optional BedrockTools event bridge can observe the three event types above. Messages submitted in the console are still saved to a local preview transcript and are **not sent to a nearby creature**.

Still not connected: full nearby-entity snapshots and identity, receiving/sending player chat events, secure AI HTTP requests, entity navigation/targeting, inventory transactions, particles, entity-attached chat bubbles, a dedicated native chat screen, multiplayer synchronization, and automatic per-world creature memory lifecycle. No API key is accepted or stored until a secure request adapter exists. Entity-affecting and network features remain disabled.

The bridge deliberately avoids guessed function offsets, vtable slots, and undocumented entity layouts. BedrockTools' own public SDK includes version-specific wrappers and signatures, but its README and LICENSE file contain inconsistent license statements; MobTalk therefore uses only the documented runtime ABI and does not bundle its source or headers. The original CreatureChat project is a Java Fabric mod; this package does not load its Java `.jar`.

## Package

The root workflow builds `mobtalk-native-arm64.levipack` separately from `levimania-mobile-arm64.levipack`.

## Licensing

The referenced CreatureChat repository identifies source code as GPL-3.0-or-later and non-code assets as CC-BY-NC-SA-4.0, with separate trademark restrictions. This native implementation does not bundle its code or protected assets. See `LICENSE-NOTICE.md`; no endorsement is implied.
