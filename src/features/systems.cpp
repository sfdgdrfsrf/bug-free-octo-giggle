#include "levimania/systems.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
namespace levimania::systems {
namespace { int idx(GridPoint p,int w){return p.z*w+p.x;} int h(GridPoint a,GridPoint b){return std::abs(a.x-b.x)+std::abs(a.z-b.z);} }
std::vector<GridPoint> findPath(const std::vector<std::uint8_t>& blocked,PathOptions o,GridPoint s,GridPoint g){
 std::vector<GridPoint> none;
 if(o.width<=0||o.height<=0||s.x<0||s.z<0||g.x<0||g.z<0||s.x>=o.width||g.x>=o.width||s.z>=o.height||g.z>=o.height||blocked.size()<static_cast<std::size_t>(o.width*o.height))return none;
 int si=idx(s,o.width),gi=idx(g,o.width); if(blocked[si]||blocked[gi])return none;
 struct N{int id;int f;}; auto cmp=[](const N&a,const N&b){return a.f>b.f;};
 std::priority_queue<N,std::vector<N>,decltype(cmp)> q(cmp);
 std::vector<int> parent(blocked.size(),-1),cost(blocked.size(),std::numeric_limits<int>::max());
 cost[si]=0;q.push({si,h(s,g)}); int visited=0; const int dx[4]={1,-1,0,0},dz[4]={0,0,1,-1};
 while(!q.empty()&&visited++<std::max(1,o.maxVisited)){auto n=q.top();q.pop();if(n.id==gi){std::vector<GridPoint> p;for(int at=gi;at!=si;at=parent[at]){if(at<0)return none;p.push_back({at%o.width,at/o.width});}p.push_back(s);std::reverse(p.begin(),p.end());return p;}
  GridPoint p{n.id%o.width,n.id/o.width};for(int k=0;k<4;k++){GridPoint v{p.x+dx[k],p.z+dz[k]};if(v.x<0||v.z<0||v.x>=o.width||v.z>=o.height)continue;int vi=idx(v,o.width),nc=cost[n.id]+1;if(blocked[vi]||nc>=cost[vi])continue;cost[vi]=nc;parent[vi]=n.id;q.push({vi,nc+h(v,g)});}}
 return none;
}
MobAdaptation adaptMob(float deaths,float kills,float damage,float difficulty){float p=std::clamp((deaths*.8f+kills*.15f+damage*.02f)/10.f,0.f,1.f),t=std::clamp(difficulty,.25f,2.f);return{std::clamp(t*(.85f+p*.3f),.5f,2.5f),std::clamp(1.f+p*.25f,.75f,1.5f),std::clamp(1.f+p*.2f,.75f,1.5f)};}
void updateCompanion(CompanionState& c,float dt,bool near){dt=std::clamp(dt,0.f,10.f);c.stamina=std::clamp(c.stamina+(near?3.f:-1.5f)*dt,0.f,100.f);if(near)c.bond=std::clamp(c.bond+.02f*dt,0.f,100.f);}
bool spendEnergy(CursedEnergy&e,float a){if(a<0||e.current<a)return false;e.current-=a;return true;}
void regenerateEnergy(CursedEnergy&e,float dt){e.current=std::clamp(e.current+std::max(0.f,dt)*e.regenerationPerSecond,0.f,e.maximum);}
bool canUseTechnique(const CursedEnergy&e,const Technique&t,float cd){return t.cost>=0&&e.current>=t.cost&&cd<=0;}
void updateMineEncounter(MineEncounter&e,float depth,float isolation,float noise,float dt){float d=std::clamp(depth,0.f,1.f),i=std::clamp(isolation,0.f,1.f),n=std::clamp(noise,0.f,1.f),step=std::clamp(dt,0.f,10.f);e.depthScore=d;e.danger=std::clamp(d*.55f+i*.3f+n*.15f,0.f,1.f);float target=std::clamp(d*.35f+i*.45f+n*.2f,0.f,1.f);e.mimicSuspicion=std::clamp(e.mimicSuspicion+(target-e.mimicSuspicion)*std::min(1.f,step*.15f),0.f,1.f);}
bool shouldRevealMimic(const MineEncounter&e){return e.mimicSuspicion>=.82f&&e.danger>=.65f;}
}
