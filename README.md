# LeviMania Mobile

Native ARM64 LeviLaunchroid mod scaffold for Minecraft Bedrock Android.

## Planned feature ports
- **Personalized-inspired:** configurable world variation and ambience profiles; rendering, biome, weather and sound changes only through APIs exposed by the installed runtime.
- **Baritone-inspired:** bounded A* grid path planner and task-planning foundation for navigation, mining and construction. Executing movement or changing blocks requires a compatible world/player bridge.
- **AdaptiveMobs-inspired:** player-performance adaptation model for aggression, health and damage scaling. Entity changes require an exposed entity API.
- **Capri Chip-inspired:** companion state foundation for bond, stamina and follow behavior. Spawning, rendering and AI require entity hooks.
- **JJK-inspired:** cursed-energy resource, regeneration and technique-cost/cooldown checks. Damage, particles and animations require combat/render hooks.
- **Jim's Mine-inspired:** depth/isolation/noise encounter director and mimic-reveal threshold. World generation, block events and mob spawning require world/entity hooks.

## Runtime safety
This is a native C++ port, not a Java/Fabric/Forge jar loader. Pure planning and simulation code is independent of Minecraft internals. No guessed offsets or unsupported Bedrock calls are used. Features requiring unavailable runtime APIs remain bridge-dependent.

## Build
GitHub Actions cross-compiles for Android ARM64 and packages the shared library with manifest.json into a .levipack.
