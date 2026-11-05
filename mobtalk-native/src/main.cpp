#include <pl/Mod.hpp>
#include <pl/ModMenu.hpp>
#include "mobtalk/mobtalk.hpp"

class MobTalkNative final {
public:
    bool load(pl::mod::ModContext& context) {
        context.logger().info("MobTalk Native loaded; runtime entity, UI and LLM adapters are not connected");
        return true;
    }
    bool enable(pl::mod::ModContext& context) {
        pl::modmenu::ModuleBuilder("mobtalk_native.core", "MobTalk Native")
            .description("Independent native creature-chat compatibility layer. Runtime features are opt-in.")
            .modId(context.id()).defaultEnabled(true)
            .config("chat_memory", "Persistent creature conversation memory (requires runtime bridge)", pl::modmenu::ConfigType::Toggle, "false")
            .config("behavior_tags", "Creature follow/flee/attack/protect behavior tags", pl::modmenu::ConfigType::Toggle, "false")
            .config("llm_requests", "External AI requests (disabled until secure HTTP adapter exists)", pl::modmenu::ConfigType::Toggle, "false")
            .config("friendship", "Per-player friendship and creature relationships", pl::modmenu::ConfigType::Toggle, "false")
            .registerModule();
        context.logger().info("MobTalk Native enabled; unsafe/unavailable integrations remain disabled");
        return true;
    }
    bool disable(pl::mod::ModContext& context) {
        pl::modmenu::unregisterModule("mobtalk_native.core");
        context.logger().info("MobTalk Native disabled");
        return true;
    }
    bool unload(pl::mod::ModContext&) { return true; }
};
PL_REGISTER_MOD(MobTalkNative, MobTalkNative{});
