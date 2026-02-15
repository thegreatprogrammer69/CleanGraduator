#ifndef CLEANGRADUATOR_UIBOOTSTRAP_H
#define CLEANGRADUATOR_UIBOOTSTRAP_H
#include <memory>

#include "ui/widgets/QtMainWindow.h"


class ViewModelsBootstrap;

class UiBootstrap {
public:
    explicit UiBootstrap(ViewModelsBootstrap& view_models);
    ~UiBootstrap();

    void initialize();

    std::unique_ptr<ui::QtMainWindow> main_window;

private:
    ViewModelsBootstrap& view_models_;
};


#endif //CLEANGRADUATOR_UIBOOTSTRAP_H