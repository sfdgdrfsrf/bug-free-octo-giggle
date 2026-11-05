# LeviMania Mobile

Native ARM64 LeviLaunchroid mod for Minecraft Bedrock Android. The deliverable is a `.levipack` containing the native shared library and manifest, not an APK or Java mod jar.

## Stability and safety

- All unfinished gameplay modules are **disabled by default**. The loader/menu scaffold can be enabled without opting into experimental gameplay.
- Pathfinding input is bounded to a maximum 262,144 cells and rejects invalid dimensions/coordinates before allocating or indexing.
- Numeric simulation inputs are checked for non-finite values and clamped to safe ranges.
- A host-native safety test suite runs before the Android package job. If tests fail, the ARM64 package job is blocked.
- CI compilation and unit tests do not replace an actual in-game smoke test. No claim of crash-free gameplay is made until the exact package is tested with the target LeviLaunchroid and Minecraft versions.

## 1:1 native port goal

The six requested ports are feature-parity targets, not loose inspirations:

- **Personalized:** UUID/seed-based client-side randomization, mob model variants, item swaps, creative inventory shuffle, block swaps, and source-style configuration.
- **Baritone:** goal types, segmented pathfinding, movement and hazard costs, chunk caching, tool-aware mining, block placement, construction/schematic assistance, follow, waypoints, settings, and commands.
- **AdaptiveMobs:** observe player actions and teach mobs to imitate crafting, mining, and block placement, with persistent adaptation.
- **Capri Chip:** installable companion chip, summon/power controls, dialogue, chat/voice requests, combat hacking, ore finding, digging, and lighting.
- **JJK / Zasatsu Bakuto:** 14 cursed techniques, domain expansions/clashes, sorcerer and curse NPCs, grades, missions, cursed tools, and cursed energy.
- **Jim's Mine:** elevator entry, multi-level copper-mine extraction, loot, extraction, gear purchases/upgrades, and escalating horror encounters.

See [PORT_PARITY.md](PORT_PARITY.md) for source references, detailed acceptance criteria, licensing constraints, and implementation status. See [RUNTIME_INTEGRATION.md](RUNTIME_INTEGRATION.md) for the verified Bedrock API surface, missing capabilities, and staged runtime integration requirements.

## Current status — foundation only

The repository now contains the first six pure-C++ gameplay engines: deterministic personalization variant selection; weighted movement costs plus bounded weighted-graph route planning; action-frequency learning for AdaptiveMobs; an offline Capri Chip request parser and bounded task queue; a 14-entry original cursed-technique registry with energy/cooldown casting; and a five-level Jim's Mine extraction/economy state machine. Regression tests cover these systems. They are still **core logic only**, not complete gameplay ports: they are not yet connected to live Bedrock rendering, entities, inventory, input, blocks, audio, or world persistence. Each feature must be connected to verified Bedrock runtime APIs and pass in-game tests before being marked complete.

## Runtime and safety constraints

This is a native C++ port, not a Java/Fabric/Forge jar loader. Java bytecode and Java-only APIs do not run inside LeviLaunchroid. Behavior must be independently implemented against verified PreLoader/LeviLaunchroid and Bedrock APIs. Do not use guessed offsets or unsupported game calls. Where a Bedrock API cannot safely provide an exact behavior, document the limitation rather than faking it.

Some reference projects have restrictive licenses. Their protected source code, assets, models, textures, sounds, and dialogue must not be copied or redistributed without permission; equivalent behavior must be independently implemented with original or authorized assets.

## Build

GitHub Actions cross-compiles both native ARM64 mods and publishes separate `levimania-mobile-arm64.levipack` and `mobtalk-native-arm64.levipack` artifacts. Each archive is verified to contain `manifest.json` and its `.so` at the ZIP root (not nested inside a directory). MobTalk currently provides a local preview console and optional BedrockTools player-action telemetry; it is not yet a live creature-chat/AI controller.
