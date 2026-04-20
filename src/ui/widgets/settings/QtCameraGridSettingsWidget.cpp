#include "QtCameraGridSettingsWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMetaObject>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>

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

QtCameraGridSettingsWidget::~QtCameraGridSettingsWidget()
{
    joinWorkerIfNeeded();
}

void QtCameraGridSettingsWidget::buildUi()
{
    auto* rootLayout = new QHBoxLayout(this);

    // Строка ввода камер


    camerasEdit_ = new QLineEdit();
    camerasEdit_->setProperty("variant", "numeric");
    camerasEdit_->setMinimumWidth(250);

    cameraCountCombo_ = new QComboBox();
    cameraCountCombo_->setMinimumWidth(100);

    const int available_count = model_.availableCameraCount();
    for (int count = 1; count <= available_count; ++count) {
        cameraCountCombo_->addItem(QString::number(count), count);
    }

    if (cameraCountCombo_->count() > 0 && model_.cameraInput.get_copy().empty()) {
        const int initial_count = cameraCountCombo_->itemData(0).toInt();
        model_.cameraInput.set(model_.cameraSequenceForCount(initial_count));
    }

    // Кнопки
    openButton_     = new QPushButton(tr("Открыть"));
    openButton_->setMinimumWidth(150);
    openButton_->setProperty("variant", "primary");

    openAllButton_  = new QPushButton(tr("Открыть все"));
    openAllButton_->setMinimumWidth(150);

    closeAllButton_ = new QPushButton(tr("Закрыть все"));
    closeAllButton_->setMinimumWidth(150);

    rootLayout->addWidget(camerasEdit_);
    rootLayout->addWidget(cameraCountCombo_);
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

    connect(cameraCountCombo_, qOverload<int>(&QComboBox::activated),
            this,
            [this](int index) {
                if (index < 0) {
                    return;
                }

                const int count = cameraCountCombo_->currentData().toInt();
                model_.cameraInput.set(model_.cameraSequenceForCount(count));
                const std::string requested = model_.cameraInput.get_copy();
                runCameraOperation([this, requested]() {
                    return model_.openForInput(requested);
                });
            });

    // Кнопки
    connect(openButton_, &QPushButton::clicked,
            this,
            [this]() {
                const std::string requested = model_.cameraInput.get_copy();
                runCameraOperation([this, requested]() {
                    return model_.openForInput(requested);
                });
            });

    connect(openAllButton_, &QPushButton::clicked,
            this,
            [this]() {
                runCameraOperation([this]() {
                    return model_.openAllIndexes();
                });
            });

    connect(closeAllButton_, &QPushButton::clicked,
            this,
            [this]() {
                runCameraOperation([this]() {
                    return model_.closeAllIndexes();
                });
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

            for (int i = 0; i < cameraCountCombo_->count(); ++i) {
                const int count = cameraCountCombo_->itemData(i).toInt();
                if (model_.cameraSequenceForCount(count) == value) {
                    if (cameraCountCombo_->currentIndex() != i) {
                        cameraCountCombo_->setCurrentIndex(i);
                    }
                    break;
                }
            }
        });
}

void QtCameraGridSettingsWidget::setBusy(bool busy)
{
    busy_ = busy;

    camerasEdit_->setEnabled(!busy_);
    cameraCountCombo_->setEnabled(!busy_);
    openButton_->setEnabled(!busy_);
    openAllButton_->setEnabled(!busy_);
    closeAllButton_->setEnabled(!busy_);
}

void QtCameraGridSettingsWidget::joinWorkerIfNeeded()
{
    if (worker_.joinable()) {
        worker_.join();
    }
}

void QtCameraGridSettingsWidget::runCameraOperation(const std::function<std::vector<int>()>& operation)
{
    if (busy_) {
        return;
    }

    joinWorkerIfNeeded();
    setBusy(true);

    worker_ = std::thread([this, operation]() {
        std::vector<int> result = operation();

        QMetaObject::invokeMethod(this, [this, result = std::move(result)]() mutable {
            model_.applyIndexes(result);
            setBusy(false);
            joinWorkerIfNeeded();
        });
    });
}
