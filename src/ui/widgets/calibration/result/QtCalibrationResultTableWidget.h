#ifndef CLEANGRADUATOR_QTCALIBRATIONRESULTTABLEWIDGET_H
#define CLEANGRADUATOR_QTCALIBRATIONRESULTTABLEWIDGET_H

#include <QTableView>
#include <memory>

#include "QtCalibrationResultTableModel.h"

namespace ui {

    class QtCalibrationResultTableWidget final : public QTableView {
        Q_OBJECT

    public:
        explicit QtCalibrationResultTableWidget(
            mvvm::CalibrationResultTableViewModel& vm,
            QWidget* parent = nullptr);

    protected:
        void resizeEvent(QResizeEvent* event) override;
        void showEvent(QShowEvent* event) override;

        QSize sizeHint() const override;

        QSize minimumSizeHint() const override;

    private:
        void setupUi();
        void connectModelSignals();

        void updateSectionSizes();

    private:
        QtCalibrationResultTableModel model_;
    };

} // namespace ui

#endif // CLEANGRADUATOR_QTCALIBRATIONRESULTTABLEWIDGET_H