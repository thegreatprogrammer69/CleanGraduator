#include "QtSettingsWidget.h"

#include <QVBoxLayout>
#include <QGroupBox>

#include "QtCameraGridSettingsWidget.h"
#include "QtInfoSettingsWidget.h"
#include "viewmodels/settings/SettingsViewModel.h"

QtSettingsWidget::QtSettingsWidget(
    mvvm::SettingsViewModel& model,
    QWidget* parent
)
    : QWidget(parent)
    , model_(model)
{
    buildUi();
    connectUi();
}

void QtSettingsWidget::buildUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(12);

    auto* infoSettingsGroup = new QGroupBox(tr("Параметры"), this);
    auto* infoSettingsLayout = new QVBoxLayout(infoSettingsGroup);
    infoSettingsLayout->setContentsMargins(8, 8, 8, 8);
    infoSettingsLayout->setSpacing(8);

    infoSettingsWidget_ = new QtInfoSettingsWidget(model_.infoSettingsViewModel(), infoSettingsGroup);
    infoSettingsLayout->addWidget(infoSettingsWidget_);
    mainLayout->addWidget(infoSettingsGroup);

    auto* cameraGridGroup = new QGroupBox(tr("Камеры"), this);

    auto* cameraGridLayout = new QVBoxLayout(cameraGridGroup);
    cameraGridLayout->setContentsMargins(8, 8, 8, 8);
    cameraGridLayout->setSpacing(8);

    cameraGridWidget_ =
        new QtCameraGridSettingsWidget(model_.cameraGridViewModel(), cameraGridGroup);

    cameraGridLayout->addWidget(cameraGridWidget_);

    mainLayout->addWidget(cameraGridGroup);

    mainLayout->addStretch();
}

void QtSettingsWidget::connectUi() {
    connect(
        cameraGridWidget_,
        &QtCameraGridSettingsWidget::crosshairAppearanceRequested,
        this,
        &QtSettingsWidget::crosshairAppearanceRequested
    );
}
