#include "QtCameraGridSettingsWidget.h"

#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSignalBlocker>
#include <QStyle>

#include "viewmodels/settings/VideoSourceGridSettingsViewModel.h"

QtCameraGridSettingsWidget::QtCameraGridSettingsWidget(
    mvvm::VideoSourceGridSettingsViewModel& model,
    QWidget* parent
)
    : QWidget(parent)
    , model_(model)
{
    buildUi();
    connectUi();
    connectViewModel();
}

QtCameraGridSettingsWidget::~QtCameraGridSettingsWidget() {

}

void QtCameraGridSettingsWidget::buildUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(8);

    // ── Row 1: Cameras ──────────────────────────────────────────────
    auto* camerasLayout = new QHBoxLayout();
    camerasLayout->setSpacing(6);

    auto* camerasLabel = new QLabel(tr("Камеры:"), this);

    camerasEdit_ = new QLineEdit(this);
    camerasEdit_->setPlaceholderText(tr("123456 или 1-6"));

    openButton_ = new QPushButton(tr("Открыть"), this);

    openAllButton_ = new QPushButton(tr("Открыть все камеры"), this);
    openAllButton_->setObjectName("openAllCamerasButton");

    camerasLayout->addWidget(camerasLabel);
    camerasLayout->addWidget(camerasEdit_, 1);
    camerasLayout->addWidget(openButton_);
    camerasLayout->addWidget(openAllButton_);

    mainLayout->addLayout(camerasLayout);

    // ── Row 2: Auto open ─────────────────────────────────────────────
    autoOpenCheckBox_ = new QCheckBox(tr("Открывать все камеры при запуске"), this);
    autoOpenCheckBox_->setToolTip(
        tr("При запуске приложения будут автоматически открыты все доступные камеры")
    );

    mainLayout->addWidget(autoOpenCheckBox_);

    // ── Row 3: Crosshair ────────────────────────────────────────────
    auto* crosshairLayout = new QHBoxLayout();
    crosshairLayout->setSpacing(6);

    drawCrosshairCheckBox_ = new QCheckBox(tr("Рисовать перекрестие"), this);

    crosshairAppearanceButton_ =
        new QPushButton(tr("Внешний вид перекрестия"), this);
    crosshairAppearanceButton_->setEnabled(false);

    crosshairLayout->addWidget(drawCrosshairCheckBox_);
    crosshairLayout->addSpacing(12);
    crosshairLayout->addWidget(crosshairAppearanceButton_);
    crosshairLayout->addStretch();

    mainLayout->addLayout(crosshairLayout);
}

void QtCameraGridSettingsWidget::connectUi() {
    connect(camerasEdit_, &QLineEdit::textChanged, this,
        [this](const QString& text) {
            auto settings = model_.settings.get_copy();
            settings.grid_string = application::dto::VideoSourceGridString(text.toStdString());
            model_.settings.set(settings);
        }
    );

    connect(openButton_, &QPushButton::clicked, this,
        [this] {
            model_.applySettings();
        }
    );

    connect(openAllButton_, &QPushButton::clicked, this,
        [this] {
            // auto settings = model_.settings.get_copy();
            // model_.settings.set(settings);

            // model_.applySettings();
        }
    );

    connect(autoOpenCheckBox_, &QCheckBox::toggled, this,
        [this](bool checked) {
            auto settings = model_.settings.get_copy();
            settings.open_cameras_at_startup = checked;
            model_.settings.set(settings);
        }
    );

    connect(drawCrosshairCheckBox_, &QCheckBox::toggled, this,
        [this](bool checked) {
            auto settings = model_.settings.get_copy();
            settings.crosshair.visible = checked;
            model_.settings.set(settings);

            crosshairAppearanceButton_->setEnabled(checked);
        }
    );

    connect(crosshairAppearanceButton_, &QPushButton::clicked,
        this, &QtCameraGridSettingsWidget::crosshairAppearanceRequested
    );
}

void QtCameraGridSettingsWidget::connectViewModel() {
    settingsSub_ = model_.settings.subscribe(
        [this](const auto& settings) {
            QSignalBlocker b1(camerasEdit_);
            QSignalBlocker b2(autoOpenCheckBox_);
            QSignalBlocker b3(drawCrosshairCheckBox_);

            camerasEdit_->setText(QString::fromStdString(settings.new_value.grid_string.toString()));
            autoOpenCheckBox_->setChecked(settings.new_value.open_cameras_at_startup);
            drawCrosshairCheckBox_->setChecked(settings.new_value.crosshair.visible);

            crosshairAppearanceButton_->setEnabled(settings.new_value.crosshair.visible);
        }
    );

    errorSub_ = model_.error.subscribe(
        [this](const auto& error) {
            const bool hasError = !error.new_value.empty();

            camerasEdit_->setProperty("error", hasError);
            camerasEdit_->setToolTip(
                hasError ? QString::fromStdString(error.new_value) : QString()
            );

            camerasEdit_->style()->polish(camerasEdit_);
        }
    );
}
