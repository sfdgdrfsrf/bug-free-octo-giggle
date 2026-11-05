#pragma once

#include <string>
#include <vector>

namespace levimania::features {
struct FeatureInfo {
    std::string id;
    std::string name;
    std::string status;
};

const std::vector<FeatureInfo>& planned_features();
}
