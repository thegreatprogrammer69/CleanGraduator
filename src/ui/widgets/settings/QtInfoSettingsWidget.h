#ifndef CLEANGRADUATOR_QTINFOSETTINGSWIDGET_H
#define CLEANGRADUATOR_QTINFOSETTINGSWIDGET_H

#include <QWidget>

#include "viewmodels/Observable.h"

namespace mvvm {
    class InfoSettingsViewModel;
}

class QComboBox;
class QCheckBox;
class QDoubleSpinBox;

class QtInfoSettingsWidget final : public QWidget {
    Q_OBJECT

public:
    explicit QtInfoSettingsWidget(
        mvvm::InfoSettingsViewModel& model,
        QWidget* parent = nullptr
    );

    ~QtInfoSettingsWidget() override;

private:
    void buildUi();
    void connectUi();
    void connectViewModel();

private:
    mvvm::InfoSettingsViewModel& model_;

    mvvm::Observable<int>::Subscription displacementSub_;
    mvvm::Observable<int>::Subscription gaugeSub_;
    mvvm::Observable<int>::Subscription precisionSub_;
    mvvm::Observable<int>::Subscription pressureUnitSub_;
    mvvm::Observable<int>::Subscription printerSub_;
    mvvm::Observable<bool>::Subscription fileLoggingEnabledSub_;
    mvvm::Observable<float>::Subscription maxCenterDeviationSub_;

    QComboBox* displacementCombo_{nullptr};
    QComboBox* gaugeCombo_{nullptr};
    QComboBox* precisionCombo_{nullptr};
    QComboBox* pressureUnitCombo_{nullptr};
    QComboBox* printerCombo_{nullptr};
    QCheckBox* fileLoggingCheckBox_{nullptr};
    QDoubleSpinBox* maxCenterDeviationSpinBox_{nullptr};
};

#endif //CLEANGRADUATOR_QTINFOSETTINGSWIDGET_H
