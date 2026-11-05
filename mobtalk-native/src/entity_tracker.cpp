#include "mobtalk/entity_tracker.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace mobtalk::entities {
namespace {
float distanceSquared(const EntitySnapshot& e,float x,float y,float z) noexcept {
    const float dx=e.x-x,dy=e.y-y,dz=e.z-z;
    return dx*dx+dy*dy+dz*dz;
}
}
void EntityTracker::update(const SnapshotFrame& frame,float px,float py,float pz,float radius) noexcept {
    if(!std::isfinite(px)||!std::isfinite(py)||!std::isfinite(pz)||!std::isfinite(radius)||radius<=0) return;
    generation_=frame.generation;
    const float maxDistance=radius*radius;
    for(std::size_t i=0;i<count_;++i) tracked_[i].lastSeenGeneration=0;
    const auto limit=std::min(frame.count,kMaxEntitiesPerFrame);
    for(std::size_t i=0;i<limit;++i) {
        const auto& e=frame.entities[i];
        if(!e.runtimeKey||!e.typeId[0]||!std::isfinite(e.x)||!std::isfinite(e.y)||!std::isfinite(e.z)) continue;
        const float d2=distanceSquared(e,px,py,pz);
        if(d2>maxDistance) continue;
        std::size_t slot=count_;
        for(std::size_t j=0;j<count_;++j) if(tracked_[j].runtimeKey==e.runtimeKey) { slot=j; break; }
        if(slot==count_) {
            if(count_==tracked_.size()) {
                slot=0;
                for(std::size_t j=1;j<count_;++j)
                    if(tracked_[j].distanceSquared>tracked_[slot].distanceSquared) slot=j;
                if(d2>=tracked_[slot].distanceSquared) continue;
            } else ++count_;
            tracked_[slot]=TrackedCreature{};
            tracked_[slot].sessionId=nextSessionId_++;
            if(nextSessionId_==0) nextSessionId_=1;
            tracked_[slot].runtimeKey=e.runtimeKey;
        }
        auto& t=tracked_[slot];
        t.runtimeKey=e.runtimeKey;
        std::memcpy(t.typeId,e.typeId,sizeof(t.typeId));
        std::memcpy(t.displayName,e.displayName,sizeof(t.displayName));
        t.x=e.x;t.y=e.y;t.z=e.z;t.yaw=e.yaw;t.pitch=e.pitch;
        t.distanceSquared=d2;t.lastSeenGeneration=static_cast<std::uint32_t>(frame.generation);
        t.isPlayer=e.isPlayer;t.isAlive=e.isAlive;
    }
    // Remove entities absent from the latest snapshot, compacting the array.
    for(std::size_t i=0;i<count_;) {
        if(tracked_[i].lastSeenGeneration==static_cast<std::uint32_t>(frame.generation)) { ++i; continue; }
        tracked_[i]=tracked_[count_-1];
        tracked_[count_-1]=TrackedCreature{};
        --count_;
    }
    std::sort(tracked_.begin(),tracked_.begin()+static_cast<std::ptrdiff_t>(count_),
        [](const auto& a,const auto& b){return a.distanceSquared<b.distanceSquared;});
}
NearbyFrame EntityTracker::nearby() const noexcept {
    NearbyFrame out{};out.generation=generation_;out.count=count_;
    for(std::size_t i=0;i<count_;++i) out.creatures[i]=tracked_[i];
    return out;
}
bool EntityTracker::find(std::uint64_t id,TrackedCreature& out) const noexcept {
    if(!id) return false;
    for(std::size_t i=0;i<count_;++i) if(tracked_[i].sessionId==id){out=tracked_[i];return true;}
    return false;
}
void EntityTracker::reset() noexcept {
    tracked_={};count_=0;generation_=0;nextSessionId_=1;
}
} // namespace mobtalk::entities
