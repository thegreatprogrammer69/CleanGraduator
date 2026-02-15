#include "OpenAllCameras.h"

#include "application/orchestrators/VideoSourceManager.h"

using namespace application::usecase;

std::vector<int> OpenAllCameras::execute()
{
    manager_.openAll();

    std::vector<int> corrected;
    for (auto idx : manager_.opened())
        corrected.push_back(idx);

    return corrected;
}
