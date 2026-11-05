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
- real PreLoader HUD draw-command overlay, on-screen Talk button, and Android keyboard input callback;
- local preview transcript save/load in the mod data directory.

The console is a genuine live PreLoader UI/input integration, but it is **not** a Bedrock chat bridge: submitted messages remain local preview text and are not delivered to entities. The current SDK's documented public surface includes lifecycle, configuration, input callbacks, HUD draw commands, on-screen buttons, and low-level hook primitives. It does not document high-level Bedrock entity enumeration, chat events, inventory transactions, navigation, world-space rendering, or multiplayer APIs.

Still missing for full CreatureChat behavior: live entity discovery and chat send/receive; secure AI HTTP adapter and key management; entity follow/flee/attack/protect navigation; inventory transactions; world-space particles and entity-attached chat bubbles; native screen UI beyond the HUD overlay; multiplayer synchronization; and automatic per-world persistence hooks. These remain disabled. No undocumented offsets are guessed and no Java/Fabric bytecode is loaded.
