#include "mobtalk/mobtalk.hpp"
#include <algorithm>
#include <cctype>
#include <limits>

namespace mobtalk {
namespace {
std::string upper(std::string_view in) {
    std::string out; out.reserve(in.size());
    for (unsigned char c : in) out.push_back(static_cast<char>(std::toupper(c)));
    return out;
}
bool parseInt(std::string_view s, int& value) {
    if (s.empty()) return false;
    int sign = 1; std::size_t i = 0;
    if (s[0] == '+' || s[0] == '-') { if (s[0] == '-') sign = -1; i = 1; }
    if (i == s.size()) return false;
    long long n = 0;
    for (; i < s.size(); ++i) {
        if (s[i] < '0' || s[i] > '9') return false;
        n = n * 10 + (s[i] - '0');
        if (n > 100000) return false;
    }
    value = static_cast<int>(n * sign); return true;
}
bool knownBehavior(std::string_view s, Behavior& out) {
    if (s=="FOLLOW") out=Behavior::Follow; else if (s=="LEAD") out=Behavior::Lead;
    else if (s=="FLEE") out=Behavior::Flee; else if (s=="ATTACK") out=Behavior::Attack;
    else if (s=="PROTECT") out=Behavior::Protect; else if (s=="UNFOLLOW") out=Behavior::Unfollow;
    else if (s=="UNLEAD") out=Behavior::Unlead; else if (s=="UNPROTECT") out=Behavior::Unprotect;
    else if (s=="UNFLEE") out=Behavior::Unflee; else if (s=="FRIENDSHIP") out=Behavior::Friendship;
    else return false;
    return true;
}
}
ParsedReply parseReply(std::string_view text) {
    ParsedReply result; result.displayText.reserve(text.size());
    for (std::size_t i=0; i<text.size();) {
        if (text[i] != '<' && text[i] != '*') { result.displayText.push_back(text[i++]); continue; }
        const char close = text[i] == '<' ? '>' : '*';
        const auto end = text.find(close, i+1);
        if (end == std::string_view::npos || end-i > 48) { result.displayText.push_back(text[i++]); continue; }
        auto token = upper(text.substr(i+1, end-i-1));
        std::size_t split = token.find_first_of(" :\\t");
        auto name = token.substr(0, split);
        Behavior behavior{};
        if (!knownBehavior(name, behavior)) { result.displayText.append(text.substr(i, end-i+1)); i=end+1; continue; }
        int argument = 0;
        bool hasArg = false;
        if (split != std::string::npos) {
            auto argText = std::string_view(token).substr(split+1);
            while (!argText.empty() && (argText.front()==' ' || argText.front()==':' || argText.front()=='\t')) argText.remove_prefix(1);
            hasArg = parseInt(argText, argument);
        }
        result.behaviors.push_back(behavior);
        if (behavior == Behavior::Friendship && hasArg) result.friendshipDelta = std::clamp(argument, -10, 10);
        i=end+1;
    }
    // Bound untrusted output and remove blank formatting artifacts.
    if (result.displayText.size() > 4096) result.displayText.resize(4096);
    return result;
}
bool appendChat(CreatureMemory& memory, ChatLine line, std::size_t maxHistory) {
    if (maxHistory == 0 || line.text.empty()) return false;
    if (line.text.size() > 2048) line.text.resize(2048);
    if (line.speaker.size() > 64) line.speaker.resize(64);
    while (memory.history.size() >= std::min<std::size_t>(maxHistory, 128)) memory.history.pop_front();
    memory.history.push_back(std::move(line));
    return true;
}
void applyReply(CreatureMemory& memory, const ParsedReply& reply) noexcept {
    memory.friendship = std::clamp(memory.friendship + reply.friendshipDelta, -100, 100);
    if (memory.conversations != std::numeric_limits<std::uint32_t>::max()) ++memory.conversations;
}
std::string buildConversationContext(const CreatureMemory& memory, std::string_view playerName,
                                     std::string_view latestMessage, std::size_t maxChars) {
    maxChars = std::clamp<std::size_t>(maxChars, 256, 16384);
    std::string out = "You are a Minecraft creature with a persistent personality. Respond in character.\n";
    out += "Creature ID: " + memory.creatureId.substr(0, 128) + "\nPersonality: " + memory.personality.substr(0, 512);
    out += "\nFriendship: " + std::to_string(std::clamp(memory.friendship, -100, 100));
    out += "\nPlayer: " + std::string(playerName.substr(0, 64)) + "\nRecent conversation:\n";
    for (const auto& line : memory.history) {
        std::string item = (line.fromPlayer ? "Player " : "Creature ") + line.speaker.substr(0,64) + ": " + line.text + "\n";
        if (out.size() + item.size() > maxChars) break;
        out += item;
    }
    std::string current = "Player: " + std::string(latestMessage.substr(0, 1024));
    if (out.size() + current.size() > maxChars) {
        if (current.size() > maxChars) current.resize(maxChars);
        if (out.size() + current.size() > maxChars) out.resize(maxChars-current.size());
    }
    out += current;
    return out;
}
Command parsePlayerCommand(std::string_view text) noexcept {
    std::string s; s.reserve(text.size());
    for (unsigned char c : text) s.push_back(static_cast<char>(std::tolower(c)));
    if (s.find("befriend") != std::string::npos || s.find("be my friend") != std::string::npos) return Command::Befriend;
    if (s.find("give") != std::string::npos || s.find("gift") != std::string::npos) return Command::GiveItem;
    if (s.find("take") != std::string::npos || s.find("steal") != std::string::npos) return Command::TakeItem;
    if (s.find("release") != std::string::npos || s.find("stop following") != std::string::npos) return Command::Release;
    if (s.find("talk") != std::string::npos || s.find("hello") != std::string::npos || s.find("hi ") != std::string::npos) return Command::Chat;
    return Command::Unknown;
}
} // namespace mobtalk
