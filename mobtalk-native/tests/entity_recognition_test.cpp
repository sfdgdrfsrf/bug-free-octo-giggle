#include "mobtalk/entity_recognition.hpp"
#include <array>
#include <cassert>
#include <cstdio>
using namespace mobtalk::entities;
int main(){
 resetRecognition();
 const std::array<EntitySample,2> samples{{
  {101,"minecraft:pig","Pig",1,0,0,0,0,false,true},
  {102,"minecraft:zombie","Zombie",3,0,0,90,0,false,true}
 }};
 assert(ingestVerifiedFrame(samples,0,0,0,8));
 auto frame=nearbyCreatures();assert(frame.count==2);
 TrackedCreature pig{};assert(findCreature(frame.creatures[0].sessionId,pig));
 assert(pig.runtimeKey==101);
 assert(!ingestVerifiedFrame(samples,0,0,0,-1));
 assert(nearbyCreatures().count==2);
 resetRecognition();assert(nearbyCreatures().count==0);
 return 0;
}
