#ifndef CLEANGRADUATOR_MAINWINDOW_H
#define CLEANGRADUATOR_MAINWINDOW_H

#include <QMainWindow>

class QDockWidget;
class QWidget;

namespace mvvm {
    class MainWindowViewModel;
    class LogViewerViewModel;
}

namespace ui {

    class QtLogViewerWidget;

    namespace widgets {
        class QtVideoSourceGridWidget;
        class QtStackPanelWidget;
    }

    class QtMainWindow final : public QMainWindow {
        Q_OBJECT
    public:
        explicit QtMainWindow(
            mvvm::MainWindowViewModel& model,
            QWidget *parent = nullptr
        );
        ~QtMainWindow() override;

    private:
        void createLogDock(mvvm::LogViewerViewModel& log_vm);

    private:
        widgets::QtVideoSourceGridWidget* m_cameras = nullptr;
        QtLogViewerWidget* log_widget_ = nullptr;
        QDockWidget* log_dock_ = nullptr;

        mvvm::MainWindowViewModel& model_;
    };
}

#endif
