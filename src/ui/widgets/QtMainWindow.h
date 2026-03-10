#ifndef CLEANGRADUATOR_MAINWINDOW_H
#define CLEANGRADUATOR_MAINWINDOW_H

#include <QMainWindow>

#include "status_bar/QtAppStatusBarWidget.h"
#include "../../viewmodels/calibration/recording/CalibrationSeriesViewModel.h"

class QDockWidget;
class QWidget;

namespace mvvm {
    class MainWindowViewModel;
    class LogViewerViewModel;
}

namespace ui {
    class QtCalibrationSeriesWidget;

    class QtLogViewerWidget;
    class QtVideoSourceGridWidget;

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

        void createCalibrationDock(mvvm::CalibrationSeriesViewModel &vm);

    private:

        QtVideoSourceGridWidget* m_cameras = nullptr;

        QtLogViewerWidget* log_widget_ = nullptr;
        QDockWidget* log_dock_ = nullptr;

        QtCalibrationSeriesWidget* calibration_widget_ = nullptr;
        QDockWidget* calibration_dock_ = nullptr;

        mvvm::MainWindowViewModel& model_;
    };
}

#endif
