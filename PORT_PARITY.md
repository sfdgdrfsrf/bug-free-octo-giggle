# Native Port Parity Specification

This document is the acceptance baseline for the six requested ports. "1:1" means matching the source mod's observable gameplay behavior, settings, progression, commands/controls, and feedback as closely as the Bedrock/LeviLaunchroid runtime permits. It does not mean loading Java bytecode or pretending Java APIs exist in Bedrock.

## Porting rules

- Reimplement behavior in native C++ against verified LeviLaunchroid/PreLoader and Minecraft Bedrock APIs.
- Do not copy proprietary source code, assets, models, textures, sounds, dialogue, or other protected content from projects whose license does not permit redistribution. Use independently authored equivalents or assets the user has rights to provide.
- Preserve source defaults and configurable options where they make sense; document every Bedrock-specific deviation.
- No guessed offsets, unsafe memory layouts, or unverified game calls. A feature is not implemented until it is connected to a real runtime bridge and passes an in-game test.
- Keep each port independently toggleable and expose settings/commands through the native mod's supported UI/command facilities.
- Track parity as: planned -> core logic -> runtime integrated -> tested -> parity verified. The current repository is at foundation/scaffold stage; no full port is complete.

## Source baselines and acceptance criteria

### Personalized
Source: https://www.curseforge.com/minecraft/mc-mods/personalized
Reference description: https://modrinth.com/mod/personalized
Source behavior to reproduce:
- Stable per-player UUID/seed-based randomization so the same player sees a consistent personalized world.
- Configurable feature toggles and seed/UUID selection.
- Randomized mob/entity model presentation (source default: off).
- Item-to-item visual/identity swaps.
- Creative inventory reshuffling.
- Block appearance/identity swaps, including the source's translucency behavior where safely supported.
- Client-side presentation: do not alter authoritative server world state merely to personalize the local view.
Bedrock deviations must be explicit where Bedrock does not expose a safe client render/item/inventory hook.

### Baritone
Source: https://github.com/cabaletta/baritone
Reference feature list: https://github.com/cabaletta/baritone/blob/1.21.4/FEATURES.md
Source behavior to reproduce:
- Goal system: block, X/Z, Y-level, get-to-block, near-entity, axis, and composite goals.
- Incremental/segmented A* path planning, chunk/world cache, time/visit limits, path splicing and replanning.
- Real movement-cost model: walk, sprint, jump/parkour, ascend/descend, fall safety, ladders/vines, water, doors/gates, slabs/stairs, and hazards.
- Tool-aware block breaking and placement costs; avoid dangerous liquid-adjacent block breaks.
- Mining process: search known/loaded/cached targets, approach, mine, collect drops, and continue/branch mine when configured.
- Construction and schematic/selection assistance, including placement order and orientation when supported by Bedrock structures.
- Follow, waypoints, process priority/cancellation, settings, chat commands, status, and debug feedback.
- Commands must include functional equivalents for goto, mine, build, follow, stop, and help.
The existing bounded four-direction grid A* is only a testable seed, not Baritone parity.

### AdaptiveMobs
Source baseline: https://www.curseforge.com/minecraft/bukkit-plugins/adaptivemobs
The selected reference describes mobs learning player actions: if a player crafts, mines, or places blocks in front of them, mobs can adapt and imitate those actions.
Source behavior to reproduce:
- Observe player actions and maintain per-world/per-player behavior memory.
- Learn and select responses to crafting, mining, and block placement rather than only scaling health/damage.
- Have mobs attempt corresponding actions using valid inventories, tools, reach, and world constraints.
- Adapt response strength/frequency over time, with persistence and reset/config controls.
- Add combat/defensive adaptations only where supported by the source behavior or explicitly documented as an extension.
This port is distinct from similarly named difficulty-scaling mods; do not silently substitute a tier-only stat scaler.

### Capri Chip
Source: https://www.curseforge.com/minecraft/mc-mods/capri-chip
Reference feature description: https://www.modpackindex.com/mod/238259/capri-chip
Source behavior to reproduce:
- Craftable/usable chip item that installs or removes the companion.
- Companion can be called/sent away; source controls include tap H and hold H for a power wheel.
- Companion presence, dialogue, contextual reactions, tricks, and optional voice interaction.
- Text chat requests and natural/simple command handling, including requests to dig tunnels, find diamonds, build walls, and light areas.
- Combat assistance using red-lightning-style hacking attacks.
- Mining/exploration assistance: locate ores, dig safely, and place lights/torches.
- Optional LLM/API key support must be opt-in, securely stored, clearable, and not required for basic offline commands/lines.
Bedrock-specific input/UI and voice permissions need native equivalents. Do not include original proprietary character art, voice, dialogue, or sounds without permission.

### Jujutsu Kaisen / Zasatsu Bakuto
Source: https://www.curseforge.com/minecraft/mc-mods/zasatsu-bakuto-jujutsu-kaisen-techniques
Source behavior to reproduce:
- All 14 cursed techniques, with each technique's distinct attacks, costs, cooldowns, targeting, and progression/unlocks.
- Domain expansions, domain clashes, and their resolution rules.
- Sorcerer and curse NPCs that use their own techniques.
- Grade progression, missions, rewards, and cursed tools.
- Cursed-energy resource management and appropriate combat feedback.
The source is marked All Rights Reserved. Implement original mechanics/code and independently created or user-authorized assets; do not extract or redistribute its jar assets/code.

### Jim's Mine
Source: https://www.curseforge.com/minecraft/mc-mods/jims-mine
Source behavior to reproduce:
- Mineshaft/shafthouse elevator entry into a separate multi-level abandoned copper-mine extraction experience.
- Five source-described levels as the initial content baseline, with progression to harder levels.
- Mine copper and gather loot, extract successfully, and use earnings to purchase better gear.
- Horror encounters/demonic entities, escalating threats, survival pressure, and level-specific challenge.
- Preserve the extraction loop: prepare -> descend -> explore/mine/loot -> survive -> extract -> upgrade -> unlock harder levels.
- Include mimic-style threats only as an additional source-aligned encounter, not as a substitute for the original extraction loop.
The source is marked All Rights Reserved. Recreate gameplay systems independently; do not redistribute its structures, art, sounds, or code without permission.

## Current repository status (2026-11-05)

- Native ARM64 shared library inside a .levipack is the target package; this is not an APK and not a Java .jar.
- Native core engines now include deterministic personalization hashing/variant selection; configurable weighted movement-cost primitives for navigation; player-action frequency memory for mob adaptation; an offline companion request parser; a 14-entry original cursed-technique metadata registry; and a five-level mine extraction/economy state machine.
- These engines are pure C++ logic and are covered by host regression tests. They are not yet connected to live Bedrock rendering, entities, inventory, input, blocks, audio, or world persistence.
- The feature registry labels the six modules `core-logic`, not runtime-integrated or parity-verified.
- No source feature should be advertised as a complete port until its runtime bridge, controls/settings, persistence, and in-game acceptance tests are implemented.
- The next engineering gate is a verified runtime capability layer (player/world/entity/block/inventory/render/input/audio/command hooks), followed by one port at a time with parity tests. If a required capability is unavailable, record the exact gap and keep that behavior disabled rather than faking it.
