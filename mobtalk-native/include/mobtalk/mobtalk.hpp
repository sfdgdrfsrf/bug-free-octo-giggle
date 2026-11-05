#pragma once
#include <cstdint>
#include <deque>
#include <string>
#include <string_view>
#include <vector>

namespace mobtalk {
enum class Behavior : std::uint8_t { Follow, Lead, Flee, Attack, Protect, Unfollow, Unlead, Unprotect, Unflee, Friendship };
struct ParsedReply { std::string displayText; std::vector<Behavior> behaviors; int friendshipDelta{0}; };
ParsedReply parseReply(std::string_view text);
struct ChatLine { bool fromPlayer{false}; std::string speaker; std::string text; std::uint64_t timestamp{0}; };
struct CreatureMemory {
    std::string creatureId;
    std::string personality;
    int friendship{0}; // -100 enemy to +100 friend
    std::uint32_t conversations{0};
    std::deque<ChatLine> history;
};
bool appendChat(CreatureMemory& memory, ChatLine line, std::size_t maxHistory = 48);
void applyReply(CreatureMemory& memory, const ParsedReply& reply) noexcept;
std::string buildConversationContext(const CreatureMemory& memory, std::string_view playerName,
                                    std::string_view latestMessage, std::size_t maxChars = 8192);
enum class Command : std::uint8_t { Unknown, Chat, Befriend, GiveItem, TakeItem, Release };
Command parsePlayerCommand(std::string_view text) noexcept;
} // namespace mobtalk
