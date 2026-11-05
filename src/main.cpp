#include <pl/Mod.hpp>
#include <pl/ModMenu.hpp>
#include "levimania/config.hpp"
#include <string>
class LeviManiaMobile final {
public:
 bool load(pl::mod::ModContext& c){ levimania::core::Config::instance().initialize(); c.logger().info("LeviMania Mobile loaded"); return true; }
 bool enable(pl::mod::ModContext& c){
  pl::modmenu::ModuleBuilder("levimania_mobile.core","LeviMania Mobile")
   .description("Native feature-port framework; world actions require supported runtime capabilities.")
   .modId(c.id()).defaultEnabled(true)
   .config("world_variation","World variation and ambience",pl::modmenu::ConfigType::Toggle,"true")
   .config("path_assist","Navigation and task planning",pl::modmenu::ConfigType::Toggle,"false")
   .config("adaptive_mobs","Adaptive mob difficulty",pl::modmenu::ConfigType::Toggle,"true")
   .config("companions","Companion systems",pl::modmenu::ConfigType::Toggle,"true")
   .config("cursed_energy","Cursed energy techniques",pl::modmenu::ConfigType::Toggle,"true")
   .config("mine_encounters","Mining encounters and mimics",pl::modmenu::ConfigType::Toggle,"true")
   .registerModule();
  c.logger().info("LeviMania Mobile enabled"); return true;
 }
 bool disable(pl::mod::ModContext& c){ pl::modmenu::unregisterModule("levimania_mobile.core"); c.logger().info("LeviMania Mobile disabled"); return true; }
 bool unload(pl::mod::ModContext&){ return true; }
};
PL_REGISTER_MOD(LeviManiaMobile, LeviManiaMobile{});
