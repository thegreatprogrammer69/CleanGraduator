#ifndef CLEANGRADUATOR_QTCALIBRATIONRESULTTABLEMODEL_H
#define CLEANGRADUATOR_QTCALIBRATIONRESULTTABLEMODEL_H

#include <optional>
#include <array>
#include <QAbstractTableModel>
#include <QVector>
#include <QString>
#include <QVariant>

#include "viewmodels/calibration/result/CalibrationResultTableViewModel.h"

namespace ui {

    struct QtCalibrationResultTableModelDeps {
        mvvm::CalibrationResultTableViewModel& vm;
    };

    class QtCalibrationResultTableModel final : public QAbstractTableModel {
        Q_OBJECT

    public:
        explicit QtCalibrationResultTableModel(
            QtCalibrationResultTableModelDeps deps,
            QObject* parent = nullptr);

        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        int columnCount(const QModelIndex& parent = QModelIndex()) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        Qt::ItemFlags flags(const QModelIndex& index) const override;

        bool isPairMergedRow(int row) const;

        bool isInfoRow(int row) const;

    private:
        enum class RowKind {
            Measurement,
            TotalAngle,
            Nonlinearity,
            CenterDeviation,
            MeasurementCount,
            CurrentAngle,
        };

        struct Cell final {
            QVariant display{};
            QString tooltip{};
            std::optional<domain::common::CalibrationIssueSeverity> max_severity{};
            std::optional<domain::common::CalibrationValidationIssueKind> validation_kind{};
            std::optional<float> nonlinearity_percent{};
            std::optional<float> center_deviation_deg{};
        };
        struct Row final {
            QVariant label{};
            std::array<Cell, 16> cells{};
            RowKind kind{RowKind::Measurement};
        };

        void applyResult(std::optional<domain::common::CalibrationResult> result);
        void applyGaugePoints(const std::vector<float>& points);
        void rebuildRows();
        void applyValidation(std::optional<domain::common::CalibrationResultValidation> validation);
        void applyInfo(const mvvm::CalibrationResultInfo& info);
        void appendInfoRows();
        void refreshRowsWithReset();
        void updateRowsInPlace();

    private:
        static constexpr int kColumnCount = 16;
        static constexpr int kSourceCount = 8;

        mvvm::CalibrationResultTableViewModel& vm_;
        mvvm::Observable<std::optional<domain::common::CalibrationResult>>::Subscription current_result_sub_;
        mvvm::Observable<std::optional<domain::common::CalibrationResultValidation>>::Subscription current_validation_sub_;
        mvvm::Observable<mvvm::CalibrationResultInfo>::Subscription current_info_sub_;
        mvvm::Observable<std::vector<float>>::Subscription current_gauge_points_sub_;

        std::optional<domain::common::CalibrationResult> current_result_;
        std::optional<domain::common::CalibrationResultValidation> current_validation_;
        mvvm::CalibrationResultInfo current_info_;
        std::vector<float> current_gauge_points_;
        QVector<Row> rows_{};
        std::array<QString, kColumnCount> horizontal_headers_{};
    };

}

#endif // CLEANGRADUATOR_QTCALIBRATIONRESULTTABLEMODEL_H
