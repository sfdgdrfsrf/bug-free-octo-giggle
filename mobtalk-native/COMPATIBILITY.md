# MobTalk Native compatibility target

## Requested game target

- Minecraft Bedrock Android: **26.60**
- Architecture: `arm64-v8a`
- Launcher: LeviLaunchroid / LeviLauncher
- Package: `mobtalk-native-arm64.levipack`

## Compatibility status (checked 2026-11-05)

**26.60 is the observed target, not yet a verified runtime target.** The user's launcher screenshot displays Minecraft v26.60 and BedrockTools v1.5.10; this confirms the displayed version, not the exact native binary build ID. Do not enable version-sensitive game hooks until the exact 26.60 Android `libminecraftpe.so` has been checked and the required signatures have been validated in-game.

Evidence reviewed:

- LeviLaunchroid v1.6.0 release notes explicitly announce Minecraft 26.60 support. They do not establish 26.60 support.
- The user's screenshot displays Minecraft v26.60. The exact installed game binary/build identifier has not been inspected.
- The user reports BedrockTools v1.5.10 in the screenshot. Its public ABI does not expose a Minecraft build identifier, and signatures/offsets remain version-specific.
- BedrockTools' event API is useful as an optional event transport, but the presence of an API ABI does not prove its underlying game signatures work on 26.60.

## Required validation before enabling live hooks

1. Confirm the installed LeviLaunchroid build can launch Minecraft 26.60 and load a minimal native `.levipack`.
2. Obtain the exact 26.60 Android game binary/build identifier from the user's installed official game.
3. Verify every required signature resolves uniquely against that exact binary; reject missing or ambiguous matches.
4. Check runtime API ABI compatibility before subscribing to BedrockTools events.
5. Validate event payload lifetimes and thread affinity. Never retain actor/player pointers beyond the documented callback lifetime.
6. Add read-only entity enumeration and chat observation first; test on a disposable world.
7. Add movement, targeting, inventory mutation, rendering, and network behavior only after each API has a verified 26.60 implementation and a rollback/disable path.

## Current fail-closed behavior

MobTalk's existing PreLoader UI, local preview transcript, bounded memory engine, and optional BedrockTools ABI event subscriptions are separate from game-version-sensitive entity hooks. Missing or incompatible runtime APIs must leave live creature interaction inactive. No guessed offsets, vtable slots, or signatures may be enabled.

The generic CMake/Android build proves only that the native library compiles for ARM64. It does **not** prove compatibility with Minecraft 26.60.
