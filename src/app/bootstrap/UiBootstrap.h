#ifndef CLEANGRADUATOR_UIBOOTSTRAP_H
#define CLEANGRADUATOR_UIBOOTSTRAP_H
#include <memory>

namespace ui {
    class QtMainWindow;
}

class ViewModelsBootstrap;

class UiBootstrap {
public:
    explicit UiBootstrap(ViewModelsBootstrap& view_models);
    ~UiBootstrap();

    void initialize();

    std::unique_ptr<ui::QtMainWindow> main_window;

private:
    ViewModelsBootstrap& view_models_;

    void createMainWindow();
};


#endif //CLEANGRADUATOR_UIBOOTSTRAP_H