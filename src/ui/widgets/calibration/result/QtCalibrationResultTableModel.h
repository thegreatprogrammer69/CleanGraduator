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
        struct Row final {
            QVariant label{};
            QVector<QVariant> values{};
        };

        void applyResult(std::optional<domain::common::CalibrationResult> result);
        void rebuildRows(const domain::common::CalibrationResult& result);

    private:
        mvvm::CalibrationResultTableViewModel& vm_;
        mvvm::Observable<std::optional<domain::common::CalibrationResult>>::Subscription current_result_sub_;

        std::optional<domain::common::CalibrationResult> current_result_;
        QVector<Row> rows_;
    };

}

#endif // CLEANGRADUATOR_QTCALIBRATIONRESULTTABLEMODEL_H