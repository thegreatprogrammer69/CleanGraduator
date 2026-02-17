#include "OpenAllCameras.h"

#include "../../orchestrators/video/VideoSourceManager.h"

using namespace application::usecase;

std::vector<int> OpenAllCameras::execute()
{
    manager_.openAll();

    std::vector<int> corrected;
    for (auto idx : manager_.opened())
        corrected.push_back(idx.value);

    return corrected;
}
