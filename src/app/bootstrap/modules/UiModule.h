#ifndef CLEANGRADUATOR_UIMODULE_H
#define CLEANGRADUATOR_UIMODULE_H

#include <memory>
#include <string>

namespace mvvm {
class MainWindowViewModel;
}

namespace ui {
class QtMainWindow;
}

struct UiModule {
    struct WindowConfig {
        int width{1100};
        int height{700};
    };

    static WindowConfig loadWindowConfig(const std::string& configDirectory);
    static std::unique_ptr<ui::QtMainWindow> createMainWindow(mvvm::MainWindowViewModel& viewModel, const WindowConfig& config);
};

#endif //CLEANGRADUATOR_UIMODULE_H
