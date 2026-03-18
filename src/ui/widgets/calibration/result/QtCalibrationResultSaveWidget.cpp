#include "QtCalibrationResultSaveWidget.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMetaObject>
#include <QUrl>
#include <QVBoxLayout>
#include <QVariant>

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
    rootLayout->setContentsMargins(12, 10, 12, 10);
    rootLayout->setSpacing(8);

    auto* topLayout = new QHBoxLayout();
    topLayout->setSpacing(8);

    batchLabel_ = new QLabel(this);
    batchLabel_->setStyleSheet("font-size: 15px; font-weight: 600;");

    stateLabel_ = new QLabel(this);
    stateLabel_->setAlignment(Qt::AlignCenter);
    stateLabel_->setMinimumWidth(120);

    topLayout->addWidget(batchLabel_);
    topLayout->addStretch(1);
    topLayout->addWidget(stateLabel_);

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(8);

    saveButton_ = new QPushButton(tr("Сохранить партию"), this);
    showInExplorerButton_ = new QPushButton(tr("Показать в проводнике"), this);
    saveAsButton_ = new QPushButton(tr("Сохранить как"), this);

    showInExplorerButton_->setProperty("secondary", true);
    saveAsButton_->setProperty("secondary", true);

    buttonLayout->addWidget(saveButton_);
    buttonLayout->addWidget(showInExplorerButton_);
    buttonLayout->addWidget(saveAsButton_);
    buttonLayout->addStretch(1);

    errorLabel_ = new QLabel(this);
    errorLabel_->setWordWrap(true);
    errorLabel_->setStyleSheet("color: #b91c1c;");

    rootLayout->addLayout(topLayout);
    rootLayout->addLayout(buttonLayout);
    rootLayout->addWidget(errorLabel_);
}

void QtCalibrationResultSaveWidget::bind()
{
    connect(saveButton_, &QPushButton::clicked, this, [this] {
        vm_.save();
    });

    connect(saveAsButton_, &QPushButton::clicked, this, [this] {
        const QString directory = QFileDialog::getExistingDirectory(
            this,
            tr("Выберите директорию для сохранения"));

        if (directory.isEmpty()) {
            return;
        }

        vm_.saveAs(directory.toStdString());
    });

    connect(showInExplorerButton_, &QPushButton::clicked, this, [this] {
        openInExplorer(vm_.last_saved_path.get_copy());
    });

    batchTextSub_ = vm_.batch_text.subscribe([this](const auto& change) {
        QMetaObject::invokeMethod(this, [this, text = QString::fromStdString(change.new_value)] {
            batchLabel_->setText(text);
        }, Qt::QueuedConnection);
    }, false);

    saveStateSub_ = vm_.save_state.subscribe([this](const auto& change) {
        const auto state = change.new_value;
        const auto text = QString::fromStdString(vm_.save_state_text.get_copy());
        QMetaObject::invokeMethod(this, [this, state, text] {
            updateStateBadge(state, text);
        }, Qt::QueuedConnection);
    }, false);

    saveStateTextSub_ = vm_.save_state_text.subscribe([this](const auto& change) {
        const auto state = vm_.save_state.get_copy();
        QMetaObject::invokeMethod(this, [this, state, text = QString::fromStdString(change.new_value)] {
            updateStateBadge(state, text);
        }, Qt::QueuedConnection);
    }, false);

    errorTextSub_ = vm_.error_text.subscribe([this](const auto& change) {
        QMetaObject::invokeMethod(this, [this, text = QString::fromStdString(change.new_value)] {
            errorLabel_->setText(text);
            errorLabel_->setVisible(!text.isEmpty());
        }, Qt::QueuedConnection);
    }, false);

    canSaveSub_ = vm_.can_save.subscribe([this](const auto& change) {
        QMetaObject::invokeMethod(this, [this, enabled = change.new_value] {
            saveButton_->setEnabled(enabled);
        }, Qt::QueuedConnection);
    }, false);

    canSaveAsSub_ = vm_.can_save_as.subscribe([this](const auto& change) {
        QMetaObject::invokeMethod(this, [this, enabled = change.new_value] {
            saveAsButton_->setEnabled(enabled);
        }, Qt::QueuedConnection);
    }, false);

    canShowInExplorerSub_ = vm_.can_show_in_explorer.subscribe([this](const auto& change) {
        QMetaObject::invokeMethod(this, [this, enabled = change.new_value] {
            showInExplorerButton_->setEnabled(enabled);
        }, Qt::QueuedConnection);
    }, false);

    batchLabel_->setText(QString::fromStdString(vm_.batch_text.get_copy()));
    updateStateBadge(vm_.save_state.get_copy(), QString::fromStdString(vm_.save_state_text.get_copy()));
    errorLabel_->setText(QString::fromStdString(vm_.error_text.get_copy()));
    errorLabel_->setVisible(!errorLabel_->text().isEmpty());
    saveButton_->setEnabled(vm_.can_save.get_copy());
    saveAsButton_->setEnabled(vm_.can_save_as.get_copy());
    showInExplorerButton_->setEnabled(vm_.can_show_in_explorer.get_copy());
}

void QtCalibrationResultSaveWidget::updateStateBadge(mvvm::CalibrationResultSaveState state, const QString& text)
{
    QString color = "#6b7280";
    QString fg = "#ffffff";

    switch (state) {
    case mvvm::CalibrationResultSaveState::Saved:
        color = "#15803d";
        break;
    case mvvm::CalibrationResultSaveState::Saving:
        color = "#b45309";
        break;
    case mvvm::CalibrationResultSaveState::Error:
        color = "#b91c1c";
        break;
    case mvvm::CalibrationResultSaveState::NotSaved:
    default:
        color = "#6b7280";
        break;
    }

    stateLabel_->setText(text);
    stateLabel_->setStyleSheet(QString(
        "background:%1; color:%2; border-radius:10px; padding:4px 10px; font-weight:600;")
        .arg(color, fg));
}

void QtCalibrationResultSaveWidget::openInExplorer(const std::filesystem::path& path)
{
    if (path.empty()) {
        return;
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(path.string())));
}

} // namespace ui
