#ifndef UI_QTCALIBRATIONGRAPHSWIDGET_H
#define UI_QTCALIBRATIONGRAPHSWIDGET_H

#include <QWidget>
#include <QColor>
#include <QPointF>

#include <optional>
#include <unordered_map>
#include <vector>

#include "viewmodels/calibration/recording/CalibrationSeriesViewModel.h"

QT_BEGIN_NAMESPACE
class QComboBox;
class QLabel;
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
        PressureVsTime,
        AngleVsTime,
        AngleVsPressure,
    };

    struct SeriesData {
        QString name;
        QColor color;
        std::vector<QPointF> points;
    };

    class PlotWidget;

    void buildUi();
    void rebuildAngleSourcesPreserveSelection();
    void refreshGraph();
    void updateSourcesVisibility();

    std::vector<int> selectedAngleSources() const;
    std::vector<SeriesData> buildSeriesForGraphType(GraphType graph_type) const;

    std::vector<QPointF> buildPressureTimePoints() const;
    std::vector<QPointF> buildAngleTimePoints(domain::common::SourceId source_id) const;
    std::vector<QPointF> buildAnglePressurePoints(domain::common::SourceId source_id) const;

    static QColor colorForSource(int source_id);
    static QString graphTitle(GraphType type);
    static QString xAxisLabel(GraphType type);
    static QString yAxisLabel(GraphType type);

    static std::optional<double> pressureAtTime(
        const std::vector<mvvm::CalibrationSeriesViewModel::PressureEntry>& pressure,
        float time);

private:
    mvvm::CalibrationSeriesViewModel& vm_;

    QComboBox* graph_type_ = nullptr;
    QListWidget* angle_sources_ = nullptr;
    QLabel* details_ = nullptr;
    PlotWidget* plot_ = nullptr;

    mvvm::Observable<mvvm::CalibrationSeriesViewModel::PressureEntry>::Subscription current_pressure_sub_;
    mvvm::Observable<std::pair<domain::common::SourceId, mvvm::CalibrationSeriesViewModel::AngleEntry>>::Subscription current_angle_sub_;
    mvvm::Observable<std::vector<domain::common::SourceId>>::Subscription source_ids_sub_;
};

} // namespace ui

#endif
