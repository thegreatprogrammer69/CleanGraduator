#pragma once
#include <QWidget>
#include <QVector>

namespace ui::presenters {
    class VideoStreamPresenter;
}

class QGridLayout;
class QFrame;

class CameraGridWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CameraGridWidget(
        ui::presenters::VideoStreamPresenter& presenter,
        int rows,
        int cols,
        double aspectRatioWH, // width/height (например 16.0/9.0)
        QWidget *parent = nullptr);

    void setAspectRatio(double aspectRatioWH);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void recalcSizes();

    int m_rows = 4;
    int m_cols = 2;
    double m_aspect = 16.0 / 9.0;

    QGridLayout *m_grid = nullptr;
    QVector<QFrame*> m_tiles;

    bool m_inRecalc = false;
};
