# LeviMania Mobile

A native ARM64 Android mod scaffold for LeviLaunchroid / LeviLauncher. This project ports feature ideas from Java mods into a native Bedrock-side implementation; it does **not** load Java .jar files.

## Target

- LeviLaunchroid native preloader SDK
- Android ARM64 (arm64-v8a)
- C++20
- Minecraft Bedrock 1.21.80+ / Android 9+ as documented by the target loader

## Current status

This is the initial project scaffold: native lifecycle entry point, config directory setup, feature roadmap registry, and an Android GitHub Actions build workflow. Feature systems are not implemented yet. The first CI run is the compatibility check for the pinned SDK and packaging steps.

## Planned feature ports

- **Personalized-inspired:** configurable world variation and ambience, limited to APIs exposed by the native runtime.
- **Baritone-inspired:** navigation, path planning, mining and construction assistance.
- **AdaptiveMobs-inspired:** mob behavior adaptation and difficulty scaling.
- **Capri Chip-inspired:** companion/creature systems.
- **JJK-inspired:** cursed-energy combat abilities and techniques.
- **Jim's Mine-inspired:** mining progression, underground encounters and mimic-style threats.

## Build

The GitHub Actions workflow builds for `arm64-v8a` and packages a `.levipack` artifact. Run it from **Actions → Build LeviMania Mobile → Run workflow**.

## Important implementation boundary

Bedrock's internal world, block, entity and player APIs are not all exposed as stable public APIs by the native loader. Each gameplay feature must be implemented only against verified SDK interfaces or a version-gated runtime bridge. Do not use guessed offsets or assume Java mod APIs are available.

## Sources

- https://docs.levimc.org/LeviLaunchroid/
- https://github.com/LiteLDev/preloader-android
