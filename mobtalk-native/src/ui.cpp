#include "mobtalk/ui.hpp"
#include "mobtalk/mobtalk.hpp"
#include "mobtalk/persistence.hpp"
#include "mobtalk/runtime_bridge.hpp"
#include "mobtalk/entity_recognition.hpp"
#include <pl/Input.hpp>
#include <pl/ModMenu.hpp>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <mutex>
#include <span>
#include <utility>
#include <string>
#include <vector>

namespace mobtalk::ui {
namespace {
struct State {
    std::mutex mutex;
    bool enabled{false};
    bool open{false};
    bool scannerOpen{false};
    bool persist{false};
    std::string draft;
    std::string status{"No creature runtime bridge connected"};
    std::filesystem::path savePath;
    CreatureMemory preview;
};
State g;

std::uint64_t nowMs() {
    return static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
}
void appendUtf8(std::string& out, std::uint32_t cp) {
    if (cp <= 0x7f) out.push_back(static_cast<char>(cp));
    else if (cp <= 0x7ff) {
        out.push_back(static_cast<char>(0xc0u | (cp >> 6)));
        out.push_back(static_cast<char>(0x80u | (cp & 0x3fu)));
    } else if (cp <= 0xffff && !(cp >= 0xd800 && cp <= 0xdfff)) {
        out.push_back(static_cast<char>(0xe0u | (cp >> 12)));
        out.push_back(static_cast<char>(0x80u | ((cp >> 6) & 0x3fu)));
        out.push_back(static_cast<char>(0x80u | (cp & 0x3fu)));
    } else if (cp <= 0x10ffff) {
        out.push_back(static_cast<char>(0xf0u | (cp >> 18)));
        out.push_back(static_cast<char>(0x80u | ((cp >> 12) & 0x3fu)));
        out.push_back(static_cast<char>(0x80u | ((cp >> 6) & 0x3fu)));
        out.push_back(static_cast<char>(0x80u | (cp & 0x3fu)));
    }
}
void eraseLastCodepoint(std::string& value) {
    if (value.empty()) return;
    std::size_t i=value.size()-1;
    while (i>0 && (static_cast<unsigned char>(value[i])&0xc0u)==0x80u) --i;
    value.resize(i);
}
void render() {
    bool visible=false, scannerVisible=false;
    std::string draft,status,last;
    {
        std::lock_guard lock(g.mutex);
        visible=g.enabled && g.open;
        scannerVisible=g.enabled && g.scannerOpen;
        draft=g.draft;
        status=g.status;
        if(!g.preview.history.empty()) last=g.preview.history.back().text;
    }
    const auto nearby=entities::nearbyCreatures();
    const auto bridge=runtime::snapshot();
    const std::string bridgeState=(bridge.bedrockToolsAvailable && bridge.subscribed)
        ? "Runtime bridge: connected" : "Runtime bridge: offline";
    const std::string eventCounts="Tick / break / place: "+std::to_string(bridge.localPlayerTicks)+"/"+
        std::to_string(bridge.blockBreakActions)+"/"+std::to_string(bridge.blockPlaceActions);
    const std::string eventCounts2="Interact / use / attack: "+std::to_string(bridge.interactActions)+"/"+
        std::to_string(bridge.itemUseActions)+"/"+std::to_string(bridge.attackEvents);
    const std::string entityProbe = bridge.actorEnumerationEnabled
        ? "Actor count probe: ON ("+std::to_string(bridge.lastActorCount)+", calls "+std::to_string(bridge.actorEnumerationCalls)+")"
        : (bridge.actorManagerListSignatureAvailable
            ? "Actor list signature found; enumeration OFF"
            : "Actor list signature unavailable");
    std::vector<pl::modmenu::DrawCommand> commands;
    if(visible || scannerVisible) {
        using Type=pl::modmenu::DrawCommandType;
        pl::modmenu::DrawCommand panel{};
        panel.type=Type::RectFilled; panel.x=24; panel.y=56; panel.w=350; panel.h=scannerVisible?344:246;
        panel.color=0xE51B1B1Bu;
        commands.push_back(panel);
        auto addText=[&](float x,float y,float size,std::string text,std::uint32_t color) {
            pl::modmenu::DrawCommand c{};
            c.type=Type::Text; c.x=x; c.y=y; c.size=size; c.color=color;
            c.text=std::move(text); commands.push_back(std::move(c));
        };
        addText(38,68,19,scannerVisible?"MobTalk Entity Scanner":"MobTalk Console",0xffffffffu);
        if(scannerVisible) {
            addText(38,91,12,"Nearby registry: "+std::to_string(nearby.count)+" entities",0xff99ff99u);
            addText(38,109,10,bridge.worldSessionSeen ? "World detected (player tick received)" : "Waiting for world/player tick",bridge.worldSessionSeen?0xff99ff99u:0xffffcc66u);
            addText(38,123,10,"Entity feed: awaiting verified enumerator",0xffffcc66u);
            const std::size_t shown=std::min<std::size_t>(nearby.count,10);
            for(std::size_t i=0;i<shown;++i) {
                const auto& mob=nearby.creatures[i];
                const std::string name=mob.displayName[0]?mob.displayName:mob.typeId;
                const std::string line=std::to_string(i+1)+". "+name+" ["+mob.typeId+"]";
                addText(38,144+static_cast<float>(i)*16,10,line,0xffffffffu);
            }
            if(nearby.count==0) addText(38,146,12,bridge.worldSessionSeen?"No mob snapshots received yet":"Join a world to start scanning",0xffccccccu);
            if(nearby.count>shown) addText(38,144+static_cast<float>(shown)*16,10,
                "+"+std::to_string(nearby.count-shown)+" more",0xffccccccu);
        } else {
            addText(38,91,12,"Preview mode - messages are not sent to mobs",0xffffcc66u);
        }
        if(!scannerVisible) {
            addText(38,111,12,bridgeState,bridge.subscribed?0xff99ff99u:0xffffcc66u);
            addText(38,129,10,eventCounts,0xffcccccc);
            addText(38,143,10,eventCounts2,0xffcccccc);
            addText(38,157,10,entityProbe,0xffffcc66u);
            addText(38,177,12,"Last local message:",0xffdddddd);
            addText(38,194,12,last.empty()?"(none yet)":last,0xffffffffu);
            addText(38,217,12,"Type below, then press Enter:",0xffdddddd);
            addText(38,234,14,draft.empty()?"Tap keyboard to type...":draft,0xffaaffaa);
            addText(38,264,10,status,0xffcccccc);
        }
    }
    pl::modmenu::submitDrawCommands("mobtalk_native.core",commands);
}
bool onKey(const pl::input::KeyEvent& event) {
    if(!event.isKeyDown) return false;
    bool handled=false,submitted=false;
    std::vector<CreatureMemory> snapshot;
    std::filesystem::path path;
    {
        std::lock_guard lock(g.mutex);
        if(!g.enabled||!g.open) return false;
        if(event.keyCode==4) {
            g.open=false; handled=true;
        } else if(event.keyCode==67) {
            eraseLastCodepoint(g.draft); handled=true;
        } else if(event.keyCode==66) {
            if(!g.draft.empty()) {
                appendChat(g.preview,ChatLine{true,"Player",g.draft,nowMs()});
                g.draft.clear();
                g.status="Local preview only - no live mob received this message";
                if(g.persist) { snapshot.push_back(g.preview); path=g.savePath; submitted=true; }
            }
            handled=true;
        } else if(event.unicodeChar>=32 && event.unicodeChar<=0x10ffffu &&
                  g.draft.size()<240) {
            appendUtf8(g.draft,event.unicodeChar); handled=true;
        }
    }
    if(submitted) {
        const bool saved=saveWorldMemory(path,snapshot);
        std::lock_guard lock(g.mutex);
        g.status=saved ? "Local preview saved - no mob received it" : "Local save failed - message remains in memory";
    }
    if(handled) render();
    return handled;
}
void onButton(std::string_view,pl::modmenu::ButtonEvent event,float) {
    if(event!=pl::modmenu::ButtonEvent::Click) return;
    bool open=false;
    {
        std::lock_guard lock(g.mutex);
        if(!g.enabled) return;
        g.open=!g.open; open=g.open;
        if(g.open) g.scannerOpen=false;
    }
    if(open) pl::input::showKeyboard();
    else pl::input::hideKeyboard();
    render();
}
void onScannerButton(std::string_view,pl::modmenu::ButtonEvent event,float) {
    if(event!=pl::modmenu::ButtonEvent::Click) return;
    {
        std::lock_guard lock(g.mutex);
        if(!g.enabled) return;
        g.scannerOpen=!g.scannerOpen;
        if(g.scannerOpen) g.open=false;
    }
    pl::input::hideKeyboard();
    render();
}
} // namespace

bool registerConsole(pl::mod::ModContext& context) {
    runtime::setActorEnumerationEnabled(false);
    {
        std::lock_guard lock(g.mutex);
        g.enabled=true; g.open=false; g.scannerOpen=false; g.persist=false; g.draft.clear();
        g.savePath=context.dataDir()/"mobtalk-local-console.mtmem";
        g.preview=CreatureMemory{};
        g.preview.creatureId="local-console-preview";
        g.preview.personality="UI preview only; not bound to a Bedrock entity";
        g.status="No live Bedrock entity/chat adapter available";
    }
    const bool module=pl::modmenu::ModuleBuilder("mobtalk_native.core","MobTalk Native")
        .description("Native conversation core and local HUD console; live entity bridge unavailable.")
        .modId(context.id()).defaultEnabled(true)
        .config("chat_memory","Save local console transcript",pl::modmenu::ConfigType::Toggle,"false")
        .config("actor_enumeration_probe","LOCKED: unsafe internal actor probe; unavailable",pl::modmenu::ConfigType::Toggle,"false")
        .onConfigChanged([](std::string_view,std::string_view key,std::string_view value) {
            if(key=="actor_enumeration_probe") {
                runtime::setActorEnumerationEnabled(value=="true" || value=="1");
            } else if(key=="chat_memory") {
                std::lock_guard lock(g.mutex);
                g.persist=(value=="true" || value=="1");
                if(g.persist) {
                    std::vector<CreatureMemory> loaded;
                    if(loadWorldMemory(g.savePath,loaded) && !loaded.empty() &&
                       loaded.front().creatureId=="local-console-preview")
                        g.preview=std::move(loaded.front());
                }
            }
        })
        .config("behavior_tags","Live creature behavior actions (requires verified Bedrock bridge)",pl::modmenu::ConfigType::Toggle,"false")
        .config("llm_requests","External AI requests (secure adapter not implemented)",pl::modmenu::ConfigType::Toggle,"false")
        .config("friendship","Live creature friendship (requires entity identity bridge)",pl::modmenu::ConfigType::Toggle,"false")
        .registerModule();
    const bool button=pl::modmenu::ButtonBuilder("mobtalk_native.open","Open MobTalk")
        .moduleId("mobtalk_native.core").modId(context.id())
        .label("Talk").behavior(pl::modmenu::ButtonBehavior::Click)
        .defaultVisible(true).onEvent(onButton).registerButton();
    const bool scannerButton=pl::modmenu::ButtonBuilder("mobtalk_native.scan","Nearby Entities")
        .moduleId("mobtalk_native.core").modId(context.id())
        .label("Scan Nearby").behavior(pl::modmenu::ButtonBehavior::Click)
        .defaultVisible(true).onEvent(onScannerButton).registerButton();
    pl::input::registerKeyCallback(onKey);
    render();
    context.logger().info("MobTalk HUD console registered; messages are local preview only");
    return module && button && scannerButton;
}
void unregisterConsole() noexcept {
    runtime::setActorEnumerationEnabled(false);
    {
        std::lock_guard lock(g.mutex);
        g.enabled=false; g.open=false; g.scannerOpen=false; g.draft.clear();
    }
    pl::input::hideKeyboard();
    pl::modmenu::submitDrawCommands("mobtalk_native.core",std::span<const pl::modmenu::DrawCommand>{});
    pl::modmenu::unregisterButton("mobtalk_native.open");
    pl::modmenu::unregisterButton("mobtalk_native.scan");
    pl::modmenu::unregisterModule("mobtalk_native.core");
}
} // namespace mobtalk::ui
