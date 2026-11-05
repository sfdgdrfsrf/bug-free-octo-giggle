#pragma once
#include <pl/Mod.hpp>

namespace mobtalk::ui {
bool registerConsole(pl::mod::ModContext& context);
void unregisterConsole() noexcept;
} // namespace mobtalk::ui
