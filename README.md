# LeviMania Mobile

Native ARM64 LeviLaunchroid mod for Minecraft Bedrock Android. The deliverable is a `.levipack` containing the native shared library and manifest, not an APK or Java mod jar.

## 1:1 native port goal

The six requested ports are being treated as feature-parity targets, not loose inspirations:

- **Personalized:** UUID/seed-based client-side randomization, mob model variants, item swaps, creative inventory shuffle, block swaps, and source-style configuration.
- **Baritone:** goal types, segmented pathfinding, movement and hazard costs, chunk caching, tool-aware mining, block placement, construction/schematic assistance, follow, waypoints, settings, and commands.
- **AdaptiveMobs:** observe player actions and teach mobs to imitate crafting, mining, and block placement, with persistent adaptation.
- **Capri Chip:** installable companion chip, summon/power controls, dialogue, chat/voice requests, combat hacking, ore finding, digging, and lighting.
- **JJK / Zasatsu Bakuto:** 14 cursed techniques, domain expansions/clashes, sorcerer and curse NPCs, grades, missions, cursed tools, and cursed energy.
- **Jim's Mine:** elevator entry, multi-level copper-mine extraction, loot, extraction, gear purchases/upgrades, and escalating horror encounters.

See [PORT_PARITY.md](PORT_PARITY.md) for source references, detailed acceptance criteria, licensing constraints, and implementation status.

## Current status — foundation only

The repository currently contains prototype data models and planning logic, not six completed gameplay ports. A basic four-direction A* routine, stat-scaling formula, companion bond/stamina model, cursed-energy resource checks, and mine encounter threshold are not considered feature parity. Every feature must be connected to verified Bedrock runtime APIs and pass in-game tests before being marked complete.

## Runtime and safety

This is a native C++ port, not a Java/Fabric/Forge jar loader. Java bytecode and Java-only APIs do not run inside LeviLaunchroid. Behavior must be independently implemented against verified PreLoader/LeviLaunchroid and Bedrock APIs. Do not use guessed offsets or unsupported game calls. Where a Bedrock API cannot safely provide an exact behavior, document the limitation rather than faking it.

Some reference projects have restrictive licenses. Their protected source code, assets, models, textures, sounds, and dialogue must not be copied or redistributed without permission; equivalent behavior must be independently implemented with original or authorized assets.

## Build

GitHub Actions cross-compiles for Android ARM64 and packages the native shared library with `manifest.json` into a `.levipack`.
