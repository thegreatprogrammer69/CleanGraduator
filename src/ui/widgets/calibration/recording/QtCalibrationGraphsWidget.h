#ifndef UI_QTCALIBRATIONGRAPHSWIDGET_H
#define UI_QTCALIBRATIONGRAPHSWIDGET_H

#include <QWidget>
#include <QColor>
#include <QString>

#include <vector>

#include "viewmodels/calibration/recording/CalibrationSeriesViewModel.h"

QT_BEGIN_NAMESPACE
class QComboBox;
class QListWidget;
class QListWidgetItem;
QT_END_NAMESPACE

namespace ui {

class QtCalibrationGraphsWidget final : public QWidget
{
    Q_OBJECT

public:
    explicit QtCalibrationGraphsWidget(mvvm::CalibrationSeriesViewModel& vm, QWidget* parent = nullptr);
    ~QtCalibrationGraphsWidget() override;

private slots:
    void onGraphTypeChanged(int index);
    void onAngleSourceItemChanged(QListWidgetItem* item);

private:
    enum class GraphType {
        PressureTime,
        AngleTime,
        AnglePressure
    };

public:
    struct PlotPoint {
        double x = 0.0;
        double y = 0.0;
    };

    struct PlotSeries {
        QString name;
        QColor color;
        std::vector<PlotPoint> points;
    };

private:
    class PlotCanvas;

    void buildUi();
    void rebuildAngleSources();
    void refreshPlot();

    std::vector<int> selectedAngleSourceIds() const;
    static QColor colorForIndex(int idx);

private:
    mvvm::CalibrationSeriesViewModel& vm_;

    QComboBox* graph_type_ = nullptr;
    QListWidget* angle_sources_ = nullptr;
    PlotCanvas* canvas_ = nullptr;

    mvvm::Observable<mvvm::CalibrationSeriesViewModel::PressureEntry>::Subscription current_pressure_sub_;
    mvvm::Observable<std::pair<domain::common::SourceId, mvvm::CalibrationSeriesViewModel::AngleEntry>>::Subscription current_angle_sub_;
    mvvm::Observable<std::vector<domain::common::SourceId>>::Subscription source_ids_sub_;
};

} // namespace ui

#endif
