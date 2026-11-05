#include <pl/Mod.hpp>
#include "mobtalk/ui.hpp"

class MobTalkNative final {
public:
    bool load(pl::mod::ModContext& context) {
        context.logger().info("MobTalk Native loaded; live Bedrock entity and AI adapters remain unavailable");
        return true;
    }
    bool enable(pl::mod::ModContext& context) {
        const bool ok=mobtalk::ui::registerConsole(context);
        context.logger().info(ok ? "MobTalk Native HUD console enabled" : "MobTalk Native HUD registration incomplete");
        return ok;
    }
    bool disable(pl::mod::ModContext&) {
        mobtalk::ui::unregisterConsole();
        return true;
    }
    bool unload(pl::mod::ModContext&) {
        mobtalk::ui::unregisterConsole();
        return true;
    }
};
PL_REGISTER_MOD(MobTalkNative, MobTalkNative{});
