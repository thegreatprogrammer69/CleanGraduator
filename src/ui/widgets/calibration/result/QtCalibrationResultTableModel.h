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

    private:
        struct Cell final {
            QVariant display{};
            QString tooltip{};
            QString base_tooltip{};
            std::optional<domain::common::CalibrationIssueSeverity> max_severity{};
            std::optional<domain::common::ClibrationResultValidation::Issues> validation_issues{};
        };
        struct Row final {
            QVariant label{};
            QVector<Cell> cells{};
        };

        void applyResult(std::optional<domain::common::CalibrationResult> result);
        void applyValidation(std::optional<domain::common::ClibrationResultValidation> validation);
        void rebuildRows(const domain::common::CalibrationResult& result);
        void rebuildValidationMarkup();

    private:
        mvvm::CalibrationResultTableViewModel& vm_;
        mvvm::Observable<std::optional<domain::common::CalibrationResult>>::Subscription current_result_sub_;
        mvvm::Observable<std::optional<domain::common::ClibrationResultValidation>>::Subscription current_validation_sub_;

        std::optional<domain::common::CalibrationResult> current_result_;
        std::optional<domain::common::ClibrationResultValidation> current_validation_;
        QVector<Row> rows_;
    };

}

#endif // CLEANGRADUATOR_QTCALIBRATIONRESULTTABLEMODEL_H
