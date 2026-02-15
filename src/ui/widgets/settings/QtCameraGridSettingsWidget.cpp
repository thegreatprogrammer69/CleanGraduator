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
    auto* rootLayout = new QHBoxLayout(this);

    // Строка ввода камер


    camerasEdit_ = new QLineEdit();
    camerasEdit_->setProperty("variant", "numeric");
    camerasEdit_->setMinimumWidth(250);

    // Кнопки
    openButton_     = new QPushButton(tr("Открыть"));
    openButton_->setMinimumWidth(150);
    openButton_->setProperty("variant", "primary");

    openAllButton_  = new QPushButton(tr("Открыть все"));
    openAllButton_->setMinimumWidth(150);

    closeAllButton_ = new QPushButton(tr("Закрыть все"));
    closeAllButton_->setMinimumWidth(150);

    rootLayout->addWidget(camerasEdit_);
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
