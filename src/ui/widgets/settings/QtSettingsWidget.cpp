#include "QtSettingsWidget.h"

#include <QVBoxLayout>
#include <QGroupBox>

#include "QtCameraGridSettingsWidget.h"

QtSettingsWidget::QtSettingsWidget(
    QtSettingsWidgetViewModels models,
    QWidget* parent
)
    : QWidget(parent)
    , models_(models)
{
    buildUi();
    connectUi();
}

void QtSettingsWidget::buildUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(12);

    // ── Camera grid settings ────────────────────────────────────────
    auto* cameraGridGroup = new QGroupBox(tr("Камеры"), this);

    auto* cameraGridLayout = new QVBoxLayout(cameraGridGroup);
    cameraGridLayout->setContentsMargins(8, 8, 8, 8);
    cameraGridLayout->setSpacing(8);

    cameraGridWidget_ =
        new QtCameraGridSettingsWidget(models_.cameraGridSettings, cameraGridGroup);

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
