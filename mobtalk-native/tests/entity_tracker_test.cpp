#include "mobtalk/entity_tracker.hpp"
#include <array>
#include <cassert>
#include <cstdio>
using namespace mobtalk::entities;
int main(){
 SnapshotFrame f{};f.generation=1;f.count=3;
 f.entities[0].runtimeKey=11;std::snprintf(f.entities[0].typeId,kTypeIdCapacity,"minecraft:pig");
 std::snprintf(f.entities[0].displayName,kNameCapacity,"Pig");f.entities[0].x=2;f.entities[0].isAlive=true;
 f.entities[1].runtimeKey=12;std::snprintf(f.entities[1].typeId,kTypeIdCapacity,"minecraft:zombie");
 std::snprintf(f.entities[1].displayName,kNameCapacity,"Zombie");f.entities[1].x=5;f.entities[1].isAlive=true;
 f.entities[2].runtimeKey=13;std::snprintf(f.entities[2].typeId,kTypeIdCapacity,"minecraft:cow");
 f.entities[2].x=50;f.entities[2].isAlive=true;
 EntityTracker tracker;tracker.update(f,0,0,0,10);
 auto near=tracker.nearby();assert(near.count==2);assert(near.creatures[0].runtimeKey==11);
 TrackedCreature target{};assert(tracker.find(near.creatures[0].sessionId,target));assert(target.typeId[10]=='p');
 const auto oldId=target.sessionId;
 SnapshotFrame next{};next.generation=2;next.count=1;next.entities[0]=f.entities[1];
 tracker.update(next,0,0,0,10);assert(tracker.nearby().count==1);
 assert(!tracker.find(oldId,target));
 tracker.reset();assert(tracker.nearby().count==0);
 return 0;
}
