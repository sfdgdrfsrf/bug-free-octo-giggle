#include "mobtalk/runtime_bridge.hpp"
#include <cassert>
int main() {
    mobtalk::runtime::stop();
    const auto before = mobtalk::runtime::snapshot();
    assert(!before.subscribed);
    // Host CI has no BedrockTools runtime loaded; failure must be clean and non-fatal.
    (void)mobtalk::runtime::start();
    mobtalk::runtime::stop();
    const auto after = mobtalk::runtime::snapshot();
    assert(!after.subscribed);
    assert(after.localPlayerTicks == before.localPlayerTicks);
    return 0;
}
