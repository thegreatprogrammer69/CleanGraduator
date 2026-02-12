#ifndef CLEANGRADUATOR_QTCOMPONENTCONFIGWIDGET_H
#define CLEANGRADUATOR_QTCOMPONENTCONFIGWIDGET_H


#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLabel>

#include <memory>
#include <unordered_map>
#include <any>

#include "application/models/config_parameter/ConfigParameter.h"

namespace application::ports {
    struct ConfigParameter;
}

namespace mvvm {
    class ComponentConfigViewModel;
}


class QtComponentConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QtComponentConfigWidget(
        mvvm::ComponentConfigViewModel& viewModel,
        QWidget* parent = nullptr
    );

    ~QtComponentConfigWidget() override = default;

private:
    void buildForm(const application::models::ConfigSchema& schema);
    QWidget* createEditor(const application::models::ConfigParameter& param);

private:
    mvvm::ComponentConfigViewModel& viewModel_;

    QVBoxLayout* rootLayout_;
    QGridLayout* gridLayout_;
    QPushButton* reloadButton_;
};



#endif //CLEANGRADUATOR_QTCOMPONENTCONFIGWIDGET_H