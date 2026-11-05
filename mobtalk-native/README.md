# MobTalk Native

A separate LeviLaunchroid `.levipack` target for an independently authored native creature-conversation layer inspired by the documented feature set of CreatureChat.

## Implemented

- Per-creature identity/personality, bounded conversation history, friendship, and allow-listed behavior-marker parsing.
- Bounded conversation-context construction and player-intent classification.
- Bounded binary memory storage with version/size checks and failure-safe loading.
- A native HUD console using the documented PreLoader ModMenu draw-command API.
- An on-screen **Talk** button and Android keyboard input callback.
- Enter submits a message to the local preview transcript; Backspace edits; Escape closes the console.
- Local transcript reload/save under the mod data directory.
- Host tests for conversation and storage logic.

## Important live-runtime boundary

This is **not yet a complete CreatureChat port**. The HUD and input console are connected to real PreLoader APIs, but messages are only saved to a local preview transcript. They are **not sent to a nearby creature** because the pinned PreLoader Android SDK 0.2.2 does not expose documented Bedrock entity enumeration or chat-event APIs.

Still not connected: live Bedrock entity discovery, receiving/sending game chat events, secure AI HTTP requests, entity navigation/targeting, inventory transactions, world-space particles, entity-attached chat bubbles, multiplayer synchronization, and automatic per-world memory lifecycle. The HUD console is an on-screen overlay, not a native Minecraft screen or world-space entity renderer. All entity-affecting and network features remain disabled. No API key is accepted or stored until a secure request adapter exists.

PreLoader 0.2.2 exposes lifecycle, configuration, input callbacks, HUD draw commands, on-screen buttons, and low-level hook primitives. It does not expose documented high-level Bedrock entity, inventory, navigation, or multiplayer APIs. No game offsets or undocumented entity layouts are guessed. The original CreatureChat project is a Java Fabric mod; this package does not load its Java `.jar`.

## Package

The root workflow builds `mobtalk-native-arm64.levipack` separately from `levimania-mobile-arm64.levipack`.

## Licensing

The referenced repository identifies source code as GPL-3.0-or-later and non-code assets as CC-BY-NC-SA-4.0, with separate trademark restrictions. This native implementation does not bundle its code or protected assets. See `LICENSE-NOTICE.md`; no endorsement is implied.
