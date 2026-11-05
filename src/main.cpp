#include <pl/Mod.hpp>

#include "levimania/config.hpp"

class LeviManiaMobile final : public pl::Mod {
public:
    void onLoad() override {
        levimania::core::Config::instance().initialize();
        getLogger().info("LeviMania Mobile loaded");
    }

    void onEnable() override {
        getLogger().info("LeviMania Mobile enabled");
    }

    void onDisable() override {
        getLogger().info("LeviMania Mobile disabled");
    }
};

PL_REGISTER_MOD(LeviManiaMobile, LeviManiaMobile::instance());
