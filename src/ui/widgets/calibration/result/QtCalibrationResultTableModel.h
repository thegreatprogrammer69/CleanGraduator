#ifndef CLEANGRADUATOR_QTCALIBRATIONRESULTTABLEMODEL_H
#define CLEANGRADUATOR_QTCALIBRATIONRESULTTABLEMODEL_H

#include <optional>
#include <unordered_map>
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

    private:
        struct Cell final {
            QVariant display{};
            QString tooltip{};
            std::optional<domain::common::CalibrationIssueSeverity> max_severity{};
            std::optional<domain::common::CalibrationIssueSeverity> validation_severity{};
        };
        struct Row final {
            QVariant label{};
            QVector<Cell> cells{};
        };

        static constexpr int kCameraCount = 8;
        static constexpr int kColumnsPerCamera = 2;
        static constexpr int kColumnCount = kCameraCount * kColumnsPerCamera;

        void applyResult(std::optional<domain::common::CalibrationResult> result);
        void rebuildRows(const domain::common::CalibrationResult& result);
        void applyValidation(const std::optional<domain::common::CalibrationResultValidation>& validation);
        void applyInfo(const mvvm::CalibrationResultInfo& info);

        Row buildSpanRow(
            const QString& label,
            const std::unordered_map<domain::common::SourceId, QString>& values) const;
        static QString formatMeasurementCount(
            const std::unordered_map<domain::common::SourceId, int>& counts,
            domain::common::SourceId source_id);
        static std::optional<float> computeScaleSpan(
            const domain::common::CalibrationResult& result,
            domain::common::SourceId source_id);
        static std::optional<float> computeScaleNonlinearity(
            const domain::common::CalibrationResult& result,
            domain::common::SourceId source_id);

    private:
        mvvm::CalibrationResultTableViewModel& vm_;
        mvvm::Observable<mvvm::CalibrationResultInfo>::Subscription info_sub_;
        mvvm::Observable<std::optional<domain::common::CalibrationResult>>::Subscription current_result_sub_;
        mvvm::Observable<std::optional<domain::common::CalibrationResultValidation>>::Subscription current_validation_sub_;

        mvvm::CalibrationResultInfo current_info_{};
        std::optional<domain::common::CalibrationResult> current_result_;
        std::optional<domain::common::CalibrationResultValidation> current_validation_;
        QVector<Row> rows_;
    };

}

#endif // CLEANGRADUATOR_QTCALIBRATIONRESULTTABLEMODEL_H
