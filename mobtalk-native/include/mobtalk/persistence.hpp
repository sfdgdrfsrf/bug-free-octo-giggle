#pragma once
#include "mobtalk/mobtalk.hpp"
#include <filesystem>
#include <vector>

namespace mobtalk {
// Storage is scoped to a stable opaque world identifier. The identifier is
// hashed into a filename; it is never used as a path component.
std::filesystem::path worldMemoryPath(const std::filesystem::path& dataDir,
                                      std::string_view worldId);
bool saveWorldMemory(const std::filesystem::path& file,
                     const std::vector<CreatureMemory>& memories) noexcept;
bool loadWorldMemory(const std::filesystem::path& file,
                     std::vector<CreatureMemory>& memories) noexcept;
} // namespace mobtalk
