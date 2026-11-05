# MobTalk Native

A separate LeviLaunchroid `.levipack` target for a native creature-conversation compatibility layer inspired by the publicly documented feature set of CreatureChat.

## Implemented native core

- Per-creature identity/personality and bounded conversation history.
- Per-creature friendship score clamped to -100..100.
- Bounded conversation-context construction for a future LLM adapter.
- Safe parsing of behavior markers: FOLLOW, LEAD, FLEE, ATTACK, PROTECT, UNFOLLOW, UNLEAD, UNPROTECT, UNFLEE, FRIENDSHIP.
- Basic player-intent classification for chat, befriend, give/take item, and release.
- Host regression tests with strict compiler warnings.

## Not yet connected

This is **not yet a complete CreatureChat port**. The current native PreLoader module does not yet connect the core to live Bedrock entity enumeration, chat events, AI HTTP requests, entity navigation/targeting, inventory transactions, particles, chat bubbles, GUI screens, multiplayer synchronization, or automatic persistent-memory lifecycle hooks. A bounded persistent world-memory file engine has been added, but it is not yet fed by live creature events or loaded/saved against an identified in-game world. Those switches therefore remain disabled by default. No API key is accepted or stored until a secure request adapter exists.

The pinned PreLoader Android SDK (0.2.2) exposes lifecycle, configuration, input callbacks, and low-level hook primitives, but not documented high-level Bedrock entity, chat, inventory, navigation, rendering, or multiplayer APIs. No game offsets or undocumented entity layouts are guessed. The original CreatureChat project is a Java Fabric mod, not a Bedrock native plugin; this package does not load the Java `.jar`.

## Package

The root GitHub Actions workflow builds this target separately as `mobtalk-native-arm64.levipack`. The main LeviMania package remains separate as `levimania-mobile-arm64.levipack`.

## Licensing and assets

The referenced source repository identifies source code as GPL-3.0-or-later and non-code assets as CC-BY-NC-SA-4.0, with separate trademark restrictions. This native implementation is independently authored and does not bundle its code, prompts, textures, screenshots, logo, or branding. See `LICENSE-NOTICE.md`. No endorsement is implied.

## Safety

All chat and context buffers are bounded. Behavior markers are allow-listed and parsed as data; they never directly invoke game actions. Live entity actions remain disabled until validated Bedrock runtime adapters are implemented and tested.
