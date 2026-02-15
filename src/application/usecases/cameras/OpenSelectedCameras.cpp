#include "OpenSelectedCameras.h"

#include <vector>

#include "application/orchestrators/VideoSourceManager.h"

using namespace application::usecase;

std::vector<int> OpenSelectedCameras::execute(const std::vector<int>& indexes)
{
    manager_.open(indexes);

    std::vector<int> corrected;
    for (auto idx : manager_.opened())
        corrected.push_back(idx);

    return corrected;
}
