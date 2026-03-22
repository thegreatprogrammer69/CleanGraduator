#ifndef CLEANGRADUATOR_QTCALIBRATIONRESULTTABLEMODEL_H
#define CLEANGRADUATOR_QTCALIBRATIONRESULTTABLEMODEL_H

#include <optional>
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

        bool shouldSpanPair(int row) const;

    private:
        struct Cell final {
            QVariant display{};
            QString tooltip{};
            std::optional<domain::common::CalibrationIssueSeverity> max_severity{};
            std::optional<domain::common::CalibrationIssueSeverity> validation_severity{};
        };
        struct Row final {
            enum class Kind { Measurement, TotalAngle, Nonlinearity, Count, CurrentAngle };

            QVariant label{};
            QVector<Cell> cells{};
            Kind kind{Kind::Measurement};
        };

        void applyResult(std::optional<domain::common::CalibrationResult> result);
        void rebuildRows(const domain::common::CalibrationResult& result);
        void applyValidation(const std::optional<domain::common::CalibrationResultValidation>& validation);
        void notifySupplementalChanged();

    private:
        mvvm::CalibrationResultTableViewModel& vm_;
        mvvm::Observable<std::optional<domain::common::CalibrationResult>>::Subscription current_result_sub_;
        mvvm::Observable<std::optional<domain::common::CalibrationResultValidation>>::Subscription current_validation_sub_;
        mvvm::Observable<int>::Subscription supplemental_revision_sub_;

        std::optional<domain::common::CalibrationResult> current_result_;
        std::optional<domain::common::CalibrationResultValidation> current_validation_;
        QVector<Row> rows_;
    };

}

#endif // CLEANGRADUATOR_QTCALIBRATIONRESULTTABLEMODEL_H
