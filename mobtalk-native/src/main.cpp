#include <pl/Mod.hpp>
#include "mobtalk/ui.hpp"
#include "mobtalk/runtime_bridge.hpp"

class MobTalkNative final {
public:
    bool load(pl::mod::ModContext& context) {
        context.logger().info("MobTalk Native loaded; optional BedrockTools runtime bridge will be checked on enable");
        return true;
    }
    bool enable(pl::mod::ModContext& context) {
        const bool uiOk = mobtalk::ui::registerConsole(context);
        const bool bridgeOk = mobtalk::runtime::start();
        context.logger().info(uiOk ? "MobTalk Native HUD console enabled" : "MobTalk Native HUD registration incomplete");
        context.logger().info(bridgeOk
            ? "BedrockTools event bridge connected: player tick, game-mode action, attack"
            : "BedrockTools API not present or incompatible; live event bridge remains inactive");
        return uiOk;
    }
    bool disable(pl::mod::ModContext&) {
        mobtalk::runtime::stop();
        mobtalk::ui::unregisterConsole();
        return true;
    }
    bool unload(pl::mod::ModContext&) {
        mobtalk::runtime::stop();
        mobtalk::ui::unregisterConsole();
        return true;
    }
};
PL_REGISTER_MOD(MobTalkNative, MobTalkNative{});
