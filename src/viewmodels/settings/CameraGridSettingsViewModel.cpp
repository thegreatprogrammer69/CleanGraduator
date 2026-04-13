#include "CameraGridSettingsViewModel.h"

#include <algorithm>
#include <set>

#include "../../application/orchestrators/video/VideoSourceManager.h"
#include "../../application/usecases/cameras/OpenSelectedCameras.h"
#include "application/usecases/cameras/CloseAllCameras.h"
#include "application/usecases/cameras/OpenAllCameras.h"

using namespace mvvm;

CameraGridSettingsViewModel::CameraGridSettingsViewModel(CameraGridSettingsViewModelDeps deps)
    : deps_(deps)
{
}

void CameraGridSettingsViewModel::open()
{
    const std::string input = cameraInput.get_copy();

    std::set<int> unique_indexes;

    for (char ch : input) {
        if (ch < '0' || ch > '9') continue;
        int index = ch - '0';
        unique_indexes.insert(index);
    }

    const std::vector request(unique_indexes.begin(), unique_indexes.end());
    const std::vector<int> corrected = deps_.open_selected.execute(request);
    setIndexes(corrected);
}

void CameraGridSettingsViewModel::openAll()
{
     setIndexes(deps_.open_all.execute());
}

void CameraGridSettingsViewModel::closeAll()
{
     setIndexes(deps_.close_all.execute());
}

int CameraGridSettingsViewModel::availableCameraCount() const
{
    return static_cast<int>(deps_.video_source_manager.available().size());
}

std::string CameraGridSettingsViewModel::cameraSequenceForCount(int count) const
{
    if (count <= 0) {
        return "";
    }

    const int max_count = availableCameraCount();
    if (max_count <= 0) {
        return "";
    }

    const int safe_count = std::min(count, max_count);

    std::string output;
    output.reserve(safe_count);

    for (int idx = 1; idx <= safe_count; ++idx) {
        if (idx >= 0 && idx <= 9) {
            output.push_back(static_cast<char>('0' + idx));
        }
    }

    return output;
}

void CameraGridSettingsViewModel::setIndexes(const std::vector<int>& indexes)
{
    std::string output;
    output.reserve(indexes.size());

    for (int idx : indexes)
    {
        if (idx >= 0 && idx <= 9)
            output.push_back(static_cast<char>('0' + idx));
    }

    cameraInput.set(output);
}
