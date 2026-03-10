#ifndef UI_QTCALIBRATIONSERIESWIDGET_H
#define UI_QTCALIBRATIONSERIESWIDGET_H

#include <QWidget>

#include <optional>

#include "../../../viewmodels/calibration/recording/CalibrationSeriesViewModel.h"

QT_BEGIN_NAMESPACE
class QComboBox;
class QPushButton;
class QCheckBox;
class QLabel;
class QTableView;
class QScrollBar;
class QAbstractTableModel;
QT_END_NAMESPACE

namespace ui {

class QtCalibrationSeriesWidget final : public QWidget
{
    Q_OBJECT

public:
    explicit QtCalibrationSeriesWidget(mvvm::CalibrationSeriesViewModel& vm, QWidget* parent = nullptr);
    ~QtCalibrationSeriesWidget() override;

private slots:
    void onSourceActivated(int index);
    void clearView();

private:
    enum class SelectedKind {
        Pressure,
        Camera
    };

    struct SelectedSource {
        SelectedKind kind = SelectedKind::Pressure;
        int camera_id = -1; // valid only for Camera
    };

private:
    class SeriesTableModel;

    void buildUi();
    void reloadSources();
    void rebuildSourceComboPreserveSelection();
    void resetFromSelected();
    void appendPressure(const mvvm::CalibrationSeriesViewModel::PressureEntry& e);
    void appendAngle(int source_id_value, const mvvm::CalibrationSeriesViewModel::AngleEntry& e);
    void scrollToBottomIfFollow();

    SelectedSource currentSelection() const;
    static QString comboTextForCamera(int source_id_value);

private:
    mvvm::CalibrationSeriesViewModel& vm_;

    QComboBox* sources_ = nullptr;
    QPushButton* clear_ = nullptr;
    QCheckBox* follow_ = nullptr;
    QLabel* status_ = nullptr;
    QTableView* table_ = nullptr;

    SeriesTableModel* entries_model_ = nullptr;

    // Локальное состояние отображения: если юзер нажал "Очистить",
    // старую историю из VM повторно не показываем, пока не придут новые данные
    // или пока не сменится источник.
    bool locally_cleared_ = false;

    // Токены подписок — тип auto/конкретный зависит от твоего Observable.
    // Ниже использую decltype(...) через std::optional.
    mvvm::Observable<mvvm::CalibrationSeriesViewModel::PressureEntry>::Subscription current_pressure_sub_;
    mvvm::Observable<std::pair<domain::common::SourceId, mvvm::CalibrationSeriesViewModel::AngleEntry>>::Subscription current_angle_sub_;
    mvvm::Observable<std::vector<domain::common::SourceId>>::Subscription source_ids_sub_;
};

} // namespace ui

#endif