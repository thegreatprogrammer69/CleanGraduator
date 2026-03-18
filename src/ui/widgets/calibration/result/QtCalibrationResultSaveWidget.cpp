#include "QtCalibrationResultSaveWidget.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMetaObject>
#include <QPushButton>
#include <QUrl>
#include <QVBoxLayout>

namespace ui {

QtCalibrationResultSaveWidget::QtCalibrationResultSaveWidget(
    mvvm::CalibrationResultSaveViewModel& vm,
    QWidget* parent)
    : QWidget(parent)
    , vm_(vm)
{
    setupUi();
    bind();
}

QtCalibrationResultSaveWidget::~QtCalibrationResultSaveWidget() = default;

void QtCalibrationResultSaveWidget::setupUi()
{
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(12, 12, 12, 12);
    rootLayout->setSpacing(8);

    partyLabel_ = new QLabel(this);
    partyLabel_->setStyleSheet("font-size: 16px; font-weight: 600;");

    statusLabel_ = new QLabel(this);
    statusLabel_->setAlignment(Qt::AlignCenter);
    statusLabel_->setMinimumHeight(28);
    statusLabel_->setStyleSheet("border-radius: 8px; padding: 4px 10px; font-weight: 600;");

    errorLabel_ = new QLabel(this);
    errorLabel_->setWordWrap(true);
    errorLabel_->setStyleSheet("color: #b91c1c;");

    saveButton_ = new QPushButton(tr("Сохранить партию"), this);
    saveButton_->setProperty("variant", "primary");

    showInExplorerButton_ = new QPushButton(tr("Показать в проводнике"), this);
    showInExplorerButton_->setProperty("variant", "secondary");
    showInExplorerButton_->setProperty("size", "small");

    saveAsButton_ = new QPushButton(tr("Сохранить как"), this);
    saveAsButton_->setProperty("variant", "secondary");
    saveAsButton_->setProperty("size", "small");

    auto* secondaryButtonsLayout = new QHBoxLayout();
    secondaryButtonsLayout->setContentsMargins(0, 0, 0, 0);
    secondaryButtonsLayout->setSpacing(8);
    secondaryButtonsLayout->addWidget(showInExplorerButton_);
    secondaryButtonsLayout->addWidget(saveAsButton_);
    secondaryButtonsLayout->addStretch(1);

    rootLayout->addWidget(partyLabel_);
    rootLayout->addWidget(statusLabel_, 0, Qt::AlignLeft);
    rootLayout->addWidget(saveButton_, 0, Qt::AlignLeft);
    rootLayout->addLayout(secondaryButtonsLayout);
    rootLayout->addWidget(errorLabel_);
}

void QtCalibrationResultSaveWidget::bind()
{
    connect(saveButton_, &QPushButton::clicked, this, [this] {
        vm_.save();
    });

    connect(saveAsButton_, &QPushButton::clicked, this, [this] {
        const auto current_directory = vm_.saved_directory.get_copy();
        const QString start_dir = current_directory
            ? QString::fromStdString(current_directory->string())
            : QString();
        const QString selected_dir = QFileDialog::getExistingDirectory(
            this,
            tr("Выберите директорию для сохранения"),
            start_dir);
        if (!selected_dir.isEmpty()) {
            vm_.saveAs(selected_dir.toStdString());
        }
    });

    connect(showInExplorerButton_, &QPushButton::clicked, this, [this] {
        const auto current_directory = vm_.saved_directory.get_copy();
        if (!current_directory) {
            return;
        }

        QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(current_directory->string())));
    });

    partySub_ = vm_.party_label.subscribe([this](const auto& change) {
        QMetaObject::invokeMethod(this, [this, value = QString::fromStdString(change.new_value)] {
            partyLabel_->setText(value);
        }, Qt::QueuedConnection);
    }, false);

    statusTextSub_ = vm_.status_text.subscribe([this](const auto& change) {
        QMetaObject::invokeMethod(this, [this, value = QString::fromStdString(change.new_value)] {
            statusLabel_->setText(value);
        }, Qt::QueuedConnection);
    }, false);

    statusStateSub_ = vm_.status_state.subscribe([this](const auto& change) {
        QMetaObject::invokeMethod(this, [this, value = change.new_value] {
            updateStatusPresentation(value);
        }, Qt::QueuedConnection);
    }, false);

    errorSub_ = vm_.error_text.subscribe([this](const auto& change) {
        QMetaObject::invokeMethod(this, [this, value = QString::fromStdString(change.new_value)] {
            errorLabel_->setText(value);
            errorLabel_->setVisible(!value.isEmpty());
        }, Qt::QueuedConnection);
    }, false);

    canSaveSub_ = vm_.can_save.subscribe([this](const auto& change) {
        QMetaObject::invokeMethod(this, [this, value = change.new_value] {
            saveButton_->setEnabled(value);
        }, Qt::QueuedConnection);
    }, false);

    canShowInExplorerSub_ = vm_.can_show_in_explorer.subscribe([this](const auto& change) {
        QMetaObject::invokeMethod(this, [this, value = change.new_value] {
            showInExplorerButton_->setEnabled(value);
        }, Qt::QueuedConnection);
    }, false);

    canSaveAsSub_ = vm_.can_save_as.subscribe([this](const auto& change) {
        QMetaObject::invokeMethod(this, [this, value = change.new_value] {
            saveAsButton_->setEnabled(value);
        }, Qt::QueuedConnection);
    }, false);

    partyLabel_->setText(QString::fromStdString(vm_.party_label.get_copy()));
    statusLabel_->setText(QString::fromStdString(vm_.status_text.get_copy()));
    updateStatusPresentation(vm_.status_state.get_copy());
    errorLabel_->setText(QString::fromStdString(vm_.error_text.get_copy()));
    errorLabel_->setVisible(!errorLabel_->text().isEmpty());
    saveButton_->setEnabled(vm_.can_save.get_copy());
    showInExplorerButton_->setEnabled(vm_.can_show_in_explorer.get_copy());
    saveAsButton_->setEnabled(vm_.can_save_as.get_copy());
}

void QtCalibrationResultSaveWidget::updateStatusPresentation(mvvm::CalibrationResultSaveViewModel::SaveState state)
{
    switch (state) {
    case mvvm::CalibrationResultSaveViewModel::SaveState::Saved:
        statusLabel_->setStyleSheet("border-radius: 8px; padding: 4px 10px; font-weight: 600; background: #dcfce7; color: #166534;");
        break;
    case mvvm::CalibrationResultSaveViewModel::SaveState::Saving:
        statusLabel_->setStyleSheet("border-radius: 8px; padding: 4px 10px; font-weight: 600; background: #dbeafe; color: #1d4ed8;");
        break;
    case mvvm::CalibrationResultSaveViewModel::SaveState::Error:
        statusLabel_->setStyleSheet("border-radius: 8px; padding: 4px 10px; font-weight: 600; background: #fee2e2; color: #b91c1c;");
        break;
    case mvvm::CalibrationResultSaveViewModel::SaveState::Unsaved:
    default:
        statusLabel_->setStyleSheet("border-radius: 8px; padding: 4px 10px; font-weight: 600; background: #e5e7eb; color: #374151;");
        break;
    }
}

} // namespace ui
