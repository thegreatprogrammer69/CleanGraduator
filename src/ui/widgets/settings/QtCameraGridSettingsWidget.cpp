#include "QtCameraGridSettingsWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QMetaObject>
#include <QPointer>

#include <thread>
#include <utility>

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
                const std::string input = model_.cameraSequenceForCount(count);
                model_.cameraInput.set(input);
                runCameraActionAsync([this, input]() {
                    return model_.openForInput(input);
                });
            });

    // Кнопки
    connect(openButton_, &QPushButton::clicked,
            this,
            [this]() {
                const std::string input = model_.cameraInput.get_copy();
                runCameraActionAsync([this, input]() {
                    return model_.openForInput(input);
                });
            });

    connect(openAllButton_, &QPushButton::clicked,
            this,
            [this]() {
                runCameraActionAsync([this]() {
                    return model_.openAllRaw();
                });
            });

    connect(closeAllButton_, &QPushButton::clicked,
            this,
            [this]() {
                runCameraActionAsync([this]() {
                    return model_.closeAllRaw();
                });
            });
}

void QtCameraGridSettingsWidget::runCameraActionAsync(std::function<std::vector<int>()> task)
{
    if (cameraActionInProgress_) {
        return;
    }

    cameraActionInProgress_ = true;
    setControlsEnabled(false);

    QPointer<QtCameraGridSettingsWidget> widget_guard(this);

    std::thread([widget_guard, task = std::move(task)]() mutable {
        std::vector<int> corrected = task();
        if (!widget_guard) {
            return;
        }

        QMetaObject::invokeMethod(widget_guard, [widget_guard, corrected = std::move(corrected)]() mutable {
            if (!widget_guard) {
                return;
            }

            widget_guard->model_.cameraInput.set(widget_guard->model_.indexesToInput(corrected));
            widget_guard->cameraActionInProgress_ = false;
            widget_guard->setControlsEnabled(true);
        }, Qt::QueuedConnection);
    }).detach();
}

void QtCameraGridSettingsWidget::setControlsEnabled(bool enabled)
{
    if (camerasEdit_) {
        camerasEdit_->setEnabled(enabled);
    }

    if (cameraCountCombo_) {
        cameraCountCombo_->setEnabled(enabled);
    }

    if (openButton_) {
        openButton_->setEnabled(enabled);
    }

    if (openAllButton_) {
        openAllButton_->setEnabled(enabled);
    }

    if (closeAllButton_) {
        closeAllButton_->setEnabled(enabled);
    }
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
