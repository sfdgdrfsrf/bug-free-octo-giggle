#include "mobtalk/persistence.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <limits>
#include <string>
#include <system_error>

namespace mobtalk {
namespace {
constexpr std::array<char, 8> Magic{'M','T','M','E','M','0','1','\0'};
constexpr std::uint32_t Version = 1;
constexpr std::uint32_t MaxCreatures = 256;
constexpr std::uint32_t MaxHistory = 128;
constexpr std::uint32_t MaxId = 128;
constexpr std::uint32_t MaxPersonality = 512;
constexpr std::uint32_t MaxSpeaker = 64;
constexpr std::uint32_t MaxLine = 2048;

std::uint64_t hashId(std::string_view value) noexcept {
    std::uint64_t hash = 14695981039346656037ull;
    for (unsigned char c : value) { hash ^= c; hash *= 1099511628211ull; }
    return hash;
}
bool writeU32(std::ostream& out, std::uint32_t v) {
    for (unsigned i=0;i<4;++i) out.put(static_cast<char>((v>>(i*8))&0xffu));
    return out.good();
}
bool readU32(std::istream& in, std::uint32_t& v) {
    v=0;
    for (unsigned i=0;i<4;++i) {
        const int c=in.get(); if(c==std::char_traits<char>::eof()) return false;
        v |= static_cast<std::uint32_t>(static_cast<unsigned char>(c))<<(i*8);
    }
    return true;
}
bool writeU64(std::ostream& out, std::uint64_t v) {
    for (unsigned i=0;i<8;++i) out.put(static_cast<char>((v>>(i*8))&0xffu));
    return out.good();
}
bool readU64(std::istream& in, std::uint64_t& v) {
    v=0;
    for (unsigned i=0;i<8;++i) {
        const int c=in.get(); if(c==std::char_traits<char>::eof()) return false;
        v |= static_cast<std::uint64_t>(static_cast<unsigned char>(c))<<(i*8);
    }
    return true;
}
bool writeString(std::ostream& out, std::string_view s, std::uint32_t cap) {
    if (s.size()>cap || !writeU32(out,static_cast<std::uint32_t>(s.size()))) return false;
    out.write(s.data(),static_cast<std::streamsize>(s.size()));
    return out.good();
}
bool readString(std::istream& in, std::string& s, std::uint32_t cap) {
    std::uint32_t size=0; if(!readU32(in,size)||size>cap) return false;
    s.assign(size,'\0');
    if(size) in.read(s.data(),static_cast<std::streamsize>(size));
    return in.good();
}
bool writeMemory(std::ostream& out,const CreatureMemory& m) {
    const auto count=std::min<std::size_t>(m.history.size(),MaxHistory);
    if(!writeString(out,m.creatureId,MaxId) ||
       !writeString(out,m.personality,MaxPersonality) ||
       !writeU32(out,static_cast<std::uint32_t>(m.friendship+100)) ||
       !writeU32(out,m.conversations) ||
       !writeU32(out,static_cast<std::uint32_t>(count))) return false;
    const auto begin=m.history.size()-count;
    for(std::size_t i=begin;i<m.history.size();++i) {
        const auto& line=m.history[i];
        if(!writeU32(out,line.fromPlayer?1u:0u) ||
           !writeString(out,line.speaker,MaxSpeaker) ||
           !writeString(out,line.text,MaxLine) ||
           !writeU64(out,line.timestamp)) return false;
    }
    return true;
}
bool readMemory(std::istream& in,CreatureMemory& m) {
    std::uint32_t friendship=0,history=0;
    if(!readString(in,m.creatureId,MaxId) ||
       !readString(in,m.personality,MaxPersonality) ||
       !readU32(in,friendship) || friendship>200 ||
       !readU32(in,m.conversations) || !readU32(in,history) ||
       history>MaxHistory) return false;
    m.friendship=static_cast<int>(friendship)-100;
    for(std::uint32_t i=0;i<history;++i) {
        ChatLine line; std::uint32_t fromPlayer=0;
        if(!readU32(in,fromPlayer)||fromPlayer>1 ||
           !readString(in,line.speaker,MaxSpeaker) ||
           !readString(in,line.text,MaxLine) ||
           !readU64(in,line.timestamp)) return false;
        line.fromPlayer=fromPlayer==1;
        m.history.push_back(std::move(line));
    }
    return true;
}
}
std::filesystem::path worldMemoryPath(const std::filesystem::path& dataDir,
                                      std::string_view worldId) {
    if(dataDir.empty()||worldId.empty()) return {};
    constexpr char digits[]="0123456789abcdef";
    const auto hash=hashId(worldId);
    std::string name(16,'0');
    for(unsigned i=0;i<16;++i) name[15-i]=digits[(hash>>(i*4))&0xfu];
    return dataDir/"world-memory"/(name+".mtmem");
}
bool saveWorldMemory(const std::filesystem::path& file,
                     const std::vector<CreatureMemory>& memories) noexcept {
    try {
        if(file.empty()||memories.size()>MaxCreatures) return false;
        std::error_code ec;
        std::filesystem::create_directories(file.parent_path(),ec);
        if(ec) return false;
        auto temp=file; temp += ".tmp";
        {
            std::ofstream out(temp,std::ios::binary|std::ios::trunc);
            if(!out) return false;
            out.write(Magic.data(),static_cast<std::streamsize>(Magic.size()));
            if(!out.good()||!writeU32(out,Version)||
               !writeU32(out,static_cast<std::uint32_t>(memories.size()))) return false;
            for(const auto& memory:memories) if(!writeMemory(out,memory)) return false;
            out.flush(); if(!out.good()) return false;
        }
        std::filesystem::rename(temp,file,ec);
        if(ec) {
            std::error_code removeError;
            std::filesystem::remove(file,removeError);
            ec.clear(); std::filesystem::rename(temp,file,ec);
        }
        if(ec) { std::error_code ignored; std::filesystem::remove(temp,ignored); return false; }
        return true;
    } catch(...) { return false; }
}
bool loadWorldMemory(const std::filesystem::path& file,
                     std::vector<CreatureMemory>& memories) noexcept {
    try {
        std::ifstream in(file,std::ios::binary);
        if(!in) return false;
        std::array<char,Magic.size()> magic{};
        in.read(magic.data(),static_cast<std::streamsize>(magic.size()));
        if(!in.good()||magic!=Magic) return false;
        std::uint32_t version=0,count=0;
        if(!readU32(in,version)||version!=Version||
           !readU32(in,count)||count>MaxCreatures) return false;
        std::vector<CreatureMemory> loaded;
        loaded.reserve(count);
        for(std::uint32_t i=0;i<count;++i) {
            CreatureMemory memory;
            if(!readMemory(in,memory)) return false;
            loaded.push_back(std::move(memory));
        }
        if(in.peek()!=std::char_traits<char>::eof()) return false;
        memories=std::move(loaded);
        return true;
    } catch(...) { return false; }
}
} // namespace mobtalk
