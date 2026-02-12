#include "OpenSelectedCameras.h"

#include <vector>

#include "application/orchestrators/VideoSourceManager.h"

using namespace application::usecase;

void OpenSelectedCameras::execute(const std::string& input)
{
    std::vector<int> ids;

    for (char c : input)
    {
        if (std::isdigit(c))
            ids.push_back(c - '0');
    }

    manager_.open(ids);
}
