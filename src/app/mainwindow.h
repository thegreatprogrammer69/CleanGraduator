#pragma once
#include <QMainWindow>

namespace ui::presenters {
    class VideoStreamPresenter;
}

class CameraGridWidget;

class MainWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(ui::presenters::VideoStreamPresenter& presenter, QWidget *parent = nullptr);

private:
    CameraGridWidget *m_cameras = nullptr;
};
