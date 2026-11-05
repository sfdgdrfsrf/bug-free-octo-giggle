#include "mobtalk/mobtalk.hpp"
#include <cassert>
#include <string>

using namespace mobtalk;
int main() {
    auto parsed = parseReply("Hello <FOLLOW> friend! <FRIENDSHIP +4> <UNKNOWN>");
    assert(parsed.displayText.find("Hello") != std::string::npos);
    assert(parsed.displayText.find("<UNKNOWN>") != std::string::npos);
    assert(parsed.behaviors.size() == 2);
    assert(parsed.friendshipDelta == 4);
    CreatureMemory memory; memory.creatureId="mob-1"; memory.personality="curious";
    applyReply(memory, parsed);
    assert(memory.friendship == 4 && memory.conversations == 1);
    for (int i=0;i<60;i++) appendChat(memory, {true,"Alex","message",static_cast<unsigned long>(i)}, 48);
    assert(memory.history.size() == 48);
    auto context=buildConversationContext(memory,"Alex","How are you?",1024);
    assert(context.size() <= 1024);
    assert(context.find("How are you?") != std::string::npos);
    assert(parsePlayerCommand("please befriend me") == Command::Befriend);
    assert(parsePlayerCommand("give you a diamond") == Command::GiveItem);
    assert(parsePlayerCommand("hello there") == Command::Chat);
    assert(parsePlayerCommand("nonsense") == Command::Unknown);
    auto huge=parseReply(std::string(5000,'x'));
    assert(huge.displayText.size() == 4096);
    return 0;
}
