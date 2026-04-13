#ifndef UI_QTCALIBRATIONCHARTSWIDGET_H
#define UI_QTCALIBRATIONCHARTSWIDGET_H

#include <QWidget>

#include <unordered_map>

#include "viewmodels/calibration/recording/CalibrationSeriesViewModel.h"

QT_BEGIN_NAMESPACE
class QComboBox;
class QLabel;
class QVBoxLayout;
class QCheckBox;
QT_END_NAMESPACE

namespace ui {

class QtCalibrationChartsWidget final : public QWidget
{
    Q_OBJECT

public:
    explicit QtCalibrationChartsWidget(mvvm::CalibrationSeriesViewModel& vm, QWidget* parent = nullptr);
    ~QtCalibrationChartsWidget() override;

private slots:
    void onGraphTypeChanged(int index);

private:
    enum class GraphType {
        PressureByTime,
        AngleByTime,
        AngleByPressure
    };

    class PlotCanvas;

    void buildUi();
    void rebuildSourceSelectors();
    void updatePlot();
    bool isAngleGraph() const;
    std::vector<int> selectedAngleSourceIds() const;

private:
    mvvm::CalibrationSeriesViewModel& vm_;

    QComboBox* graph_type_ = nullptr;
    QWidget* source_panel_ = nullptr;
    QVBoxLayout* source_layout_ = nullptr;
    QLabel* source_hint_ = nullptr;
    PlotCanvas* plot_canvas_ = nullptr;

    std::vector<QCheckBox*> source_checks_;

    mvvm::Observable<mvvm::CalibrationSeriesViewModel::PressureEntry>::Subscription current_pressure_sub_;
    mvvm::Observable<std::pair<domain::common::SourceId, mvvm::CalibrationSeriesViewModel::AngleEntry>>::Subscription current_angle_sub_;
    mvvm::Observable<std::vector<domain::common::SourceId>>::Subscription source_ids_sub_;
};

} // namespace ui

#endif
