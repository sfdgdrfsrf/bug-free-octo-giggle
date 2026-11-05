#include "mobtalk/persistence.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

int main() {
    const auto root=std::filesystem::temp_directory_path()/"mobtalk-persistence-test";
    std::error_code ec; std::filesystem::remove_all(root,ec);
    const auto path=mobtalk::worldMemoryPath(root,"world-A");
    assert(!path.empty());
    assert(path!=mobtalk::worldMemoryPath(root,"world-B"));
    mobtalk::CreatureMemory creature;
    creature.creatureId="entity-123"; creature.personality="curious";
    creature.friendship=42; creature.conversations=7;
    creature.history.push_back({true,"Alex","hello",123456});
    creature.history.push_back({false,"Buddy","hey there",123457});
    assert(mobtalk::saveWorldMemory(path,{creature}));
    std::vector<mobtalk::CreatureMemory> loaded;
    assert(mobtalk::loadWorldMemory(path,loaded));
    assert(loaded.size()==1);
    assert(loaded[0].creatureId==creature.creatureId);
    assert(loaded[0].personality==creature.personality);
    assert(loaded[0].friendship==42 && loaded[0].conversations==7);
    assert(loaded[0].history.size()==2);
    assert(loaded[0].history[1].text=="hey there");
    assert(!mobtalk::saveWorldMemory(path,{})); // empty is allowed below
    assert(mobtalk::saveWorldMemory(path,std::vector<mobtalk::CreatureMemory>{}));
    loaded.clear();
    assert(mobtalk::loadWorldMemory(path,loaded) && loaded.empty());
    {
        std::ofstream corrupt(path,std::ios::binary|std::ios::trunc);
        corrupt<<"not-a-memory-file";
    }
    loaded.push_back(creature);
    assert(!mobtalk::loadWorldMemory(path,loaded));
    assert(loaded.size()==1); // failed loads never partially replace live state
    std::filesystem::remove_all(root,ec);
}
