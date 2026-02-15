#include "CloseAllCameras.h"

#include "application/orchestrators/VideoSourceManager.h"

using namespace application::usecase;

std::vector<int> CloseAllCameras::execute()
{
    manager_.closeAll();

    std::vector<int> corrected;
    for (auto idx : manager_.opened())
        corrected.push_back(idx);

    return corrected;
}
