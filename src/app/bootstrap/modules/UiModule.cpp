#include "UiModule.h"

#include <stdexcept>

#include "infrastructure/utils/ini/IniFile.h"
#include "ui/widgets/QtMainWindow.h"

UiModule::WindowConfig UiModule::loadWindowConfig(const std::string& configDirectory) {
    utils::ini::IniFile ini;
    const auto path = configDirectory + "/window.ini";
    if (!ini.load(path)) {
        throw std::runtime_error("Failed to load setup: " + path);
    }

    WindowConfig config;
    const auto& section = ini["window"];
    config.width = section.getInt("width", config.width);
    config.height = section.getInt("height", config.height);
    return config;
}

std::unique_ptr<ui::QtMainWindow> UiModule::createMainWindow(mvvm::MainWindowViewModel& viewModel, const WindowConfig& config) {
    auto window = std::make_unique<ui::QtMainWindow>(viewModel);
    window->resize(config.width, config.height);
    return window;
}
