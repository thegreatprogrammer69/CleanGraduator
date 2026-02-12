#ifndef CLEANGRADUATOR_VIEWMODELSMODULE_H
#define CLEANGRADUATOR_VIEWMODELSMODULE_H

#include <memory>
#include <vector>

namespace domain::ports {
struct IVideoSource;
}

namespace infrastructure::settings {
class IGridSettingsRepository;
}

namespace mvvm {
class VideoSourceViewModel;
class VideoSourceGridViewModel;
class MainWindowViewModel;
}

struct ViewModelsModule {
    static std::vector<std::unique_ptr<mvvm::VideoSourceViewModel>> createVideoSourceViewModels(std::vector<domain::ports::IVideoSource*> sources);
    static std::unique_ptr<mvvm::VideoSourceGridViewModel> createGridViewModel(const std::vector<std::unique_ptr<mvvm::VideoSourceViewModel>>& sourceViewModels);

};

#endif //CLEANGRADUATOR_VIEWMODELSMODULE_H
