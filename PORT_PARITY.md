# Native Port Parity Specification

The six LeviMania feature ports are tracked against their original observable gameplay behavior. "1:1" means matching behavior, settings, progression, controls, and feedback as closely as the native Bedrock runtime permits—not loading Java bytecode or pretending Java APIs exist.

## Porting rules
- Reimplement against verified LeviLaunchroid/PreLoader and Bedrock APIs.
- Do not copy protected code/assets where redistribution is not permitted.
- No guessed offsets, unsafe memory layouts, or unverified game calls.
- Track each port as planned -> core logic -> runtime integrated -> tested -> parity verified.
- Keep unfinished/experimental features opt-in and disabled by default.

## Six requested ports
- **Personalized:** stable UUID/seed-based client presentation variants, item/block appearance swaps, inventory reshuffling where supported.
- **Baritone:** goal system, incremental A*, world cache, movement costs, tool-aware mining, construction, follow, waypoints, commands.
- **AdaptiveMobs:** observe player crafting/mining/placement and teach mobs valid responses, with persistence.
- **Capri Chip:** installable companion, call/send-away controls, dialogue, offline tasks, optional secure AI/voice, mining/building assistance.
- **JJK / Zasatsu Bakuto:** fourteen original cursed techniques, energy/cooldowns, domains, NPCs, grades, missions, tools.
- **Jim's Mine:** five-level extraction loop, copper/loot, extraction, upgrades, escalating threats and optional mimics.

## Current status (2026-11-05)
The six LeviMania modules have pure native core engines and host regression tests, but they are not yet connected to live Bedrock world/entity/block/inventory/render/audio APIs. They are not full or parity-verified ports.

## Separate package: MobTalk Native
Reference: https://github.com/CreatureChat/creature-chat

Implemented native core:
- bounded per-creature memory/context and friendship;
- allow-listed behavior-marker parsing and intent classification;
- bounded binary persistence engine;
- real PreLoader HUD draw-command overlay and Android keyboard input callback;
- optional BedrockTools API ABI v1 bridge for local-player tick, game-mode action, and attack notifications;
- local preview transcript save/load.

The optional event bridge is a real runtime subscription, but not a creature-control bridge. It does not enumerate nearby entities, read entity identities, or issue movement/inventory/render commands. Submitted chat remains local preview text, not a game chat event or creature response.

Still missing for full CreatureChat behavior: nearby entity enumeration; player chat send/receive; secure AI HTTP adapter and key management; entity follow/flee/attack/protect navigation; inventory transactions; world-space particles and entity-attached chat bubbles; dedicated native chat UI; multiplayer synchronization; and automatic per-world persistence hooks. These remain disabled. The bridge fails closed if BedrockTools API ABI v1 is absent/incompatible. No undocumented offsets are guessed and no Java/Fabric bytecode is loaded.
