#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace levimania::systems {
struct WorldProfile { float fogDensity{0}; float ambienceVolume{1}; float weatherBias{0}; std::uint32_t variationSeed{0}; bool enabled{true}; };
struct GridPoint { int x{}; int z{}; };
struct PathOptions { int width{}; int height{}; int maxVisited{4096}; };
std::vector<GridPoint> findPath(const std::vector<std::uint8_t>&,PathOptions,GridPoint,GridPoint);
struct MobAdaptation { float aggression{1}; float healthScale{1}; float damageScale{1}; };
MobAdaptation adaptMob(float,float,float,float);
struct CompanionState { std::string name{"Companion"}; float bond{0}; float stamina{100}; bool following{true}; };
void updateCompanion(CompanionState&,float,bool);
struct CursedEnergy { float current{100}; float maximum{100}; float regenerationPerSecond{2}; };
bool spendEnergy(CursedEnergy&,float);
void regenerateEnergy(CursedEnergy&,float);
struct Technique { std::string id; float cost{}; float cooldownSeconds{}; };
bool canUseTechnique(const CursedEnergy&,const Technique&,float);
struct MineEncounter { float depthScore{0}; float danger{0}; float mimicSuspicion{0}; std::uint32_t encounterSeed{0}; };
void updateMineEncounter(MineEncounter&,float,float,float,float);
bool shouldRevealMimic(const MineEncounter&);
}
