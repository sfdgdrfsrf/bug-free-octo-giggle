#include <pl/Mod.hpp>
#include <pl/ModMenu.hpp>
#include "levimania/config.hpp"

class LeviManiaMobile final {
public:
    bool load(pl::mod::ModContext& context) {
        levimania::core::Config::instance().initialize();
        context.logger().info("LeviMania Mobile loaded (unfinished gameplay modules remain opt-in)");
        return true;
    }

    bool enable(pl::mod::ModContext& context) {
        pl::modmenu::ModuleBuilder("levimania_mobile.core", "LeviMania Mobile")
            .description("Native feature-port framework. Experimental gameplay modules are disabled until runtime-tested.")
            .modId(context.id()).defaultEnabled(true)
            .config("world_variation", "World variation and ambience (experimental)", pl::modmenu::ConfigType::Toggle, "false")
            .config("path_assist", "Navigation and task planning (experimental)", pl::modmenu::ConfigType::Toggle, "false")
            .config("adaptive_mobs", "Adaptive mob behaviors (experimental)", pl::modmenu::ConfigType::Toggle, "false")
            .config("companions", "Companion systems (experimental)", pl::modmenu::ConfigType::Toggle, "false")
            .config("cursed_energy", "Cursed energy techniques (experimental)", pl::modmenu::ConfigType::Toggle, "false")
            .config("mine_encounters", "Mining encounters and mimics (experimental)", pl::modmenu::ConfigType::Toggle, "false")
            .registerModule();
        context.logger().info("LeviMania Mobile enabled; experimental features default to off");
        return true;
    }

    bool disable(pl::mod::ModContext& context) {
        pl::modmenu::unregisterModule("levimania_mobile.core");
        context.logger().info("LeviMania Mobile disabled");
        return true;
    }

    bool unload(pl::mod::ModContext&) { return true; }
};

PL_REGISTER_MOD(LeviManiaMobile, LeviManiaMobile{});
