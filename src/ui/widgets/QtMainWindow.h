#ifndef CLEANGRADUATOR_MAINWINDOW_H
#define CLEANGRADUATOR_MAINWINDOW_H
#include <QMainWindow>

namespace mvvm {
    class MainWindowViewModel;
}

namespace ui {
    namespace widgets {
        class QtVideoSourceGridWidget;
        class QtStackPanelWidget;
    }

    class QtMainWindow final : public QMainWindow {
        Q_OBJECT
    public:
        explicit QtMainWindow(mvvm::MainWindowViewModel& model, QWidget *parent = nullptr);

    private:
        widgets::QtVideoSourceGridWidget *m_cameras = nullptr;
        mvvm::MainWindowViewModel& model_;
    };
}


#endif //CLEANGRADUATOR_MAINWINDOW_H