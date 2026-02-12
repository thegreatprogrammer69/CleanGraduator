#include "QtCameraGridSettingsWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

#include "viewmodels/settings/CameraGridSettingsViewModel.h"

QtCameraGridSettingsWidget::QtCameraGridSettingsWidget(
    mvvm::CameraGridSettingsViewModel& model,
    QWidget* parent
)
    : QWidget(parent)
    , model_(model)
{
    buildUi();
    connectUi();
    connectViewModel();
}

QtCameraGridSettingsWidget::~QtCameraGridSettingsWidget() = default;

void QtCameraGridSettingsWidget::buildUi()
{
    auto* rootLayout = new QVBoxLayout(this);

    // Строка ввода камер
    auto* rowLayout = new QHBoxLayout();

    auto* label = new QLabel("Cameras:");
    camerasEdit_ = new QLineEdit();

    rowLayout->addWidget(label);
    rowLayout->addWidget(camerasEdit_);

    // Кнопки
    openButton_     = new QPushButton("Open");
    openAllButton_  = new QPushButton("Open All");
    closeAllButton_ = new QPushButton("Close All");

    rootLayout->addLayout(rowLayout);
    rootLayout->addWidget(openButton_);
    rootLayout->addWidget(openAllButton_);
    rootLayout->addWidget(closeAllButton_);

    rootLayout->addStretch();
}

void QtCameraGridSettingsWidget::connectUi()
{
    // Ввод текста → Observable
    connect(camerasEdit_, &QLineEdit::textChanged,
            this,
            [this](const QString& text) {
                model_.cameraInput.set(text.toStdString());
            });

    // Кнопки
    connect(openButton_, &QPushButton::clicked,
            this,
            [this]() {
                model_.open();
            });

    connect(openAllButton_, &QPushButton::clicked,
            this,
            [this]() {
                model_.openAll();
            });

    connect(closeAllButton_, &QPushButton::clicked,
            this,
            [this]() {
                model_.closeAll();
            });
}

void QtCameraGridSettingsWidget::connectViewModel()
{
    cameraInputSub_ = model_.cameraInput.subscribe(
        [this](const auto& event) {

            const std::string& value = event.new_value;

            if (camerasEdit_->text().toStdString() != value)
            {
                camerasEdit_->setText(
                    QString::fromStdString(value)
                );
            }
        });
}
