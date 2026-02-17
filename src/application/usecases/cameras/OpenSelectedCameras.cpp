#include "OpenSelectedCameras.h"

#include <vector>

#include "../../orchestrators/video/VideoSourceManager.h"

using namespace application::usecase;
using namespace domain::common;

std::vector<int> OpenSelectedCameras::execute(const std::vector<int>& indexes)
{
    std::vector<AngleSourceId> ids(indexes.begin(), indexes.end());

    manager_.open(ids);

    const auto& opened = manager_.opened();

    std::vector<int> corrected;
    corrected.reserve(opened.size());

    for (const auto& id : opened)
        corrected.push_back(id.value);

    return corrected;

}
