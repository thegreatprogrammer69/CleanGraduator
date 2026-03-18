#include "QtCalibrationResultSaveWidget.h"

#include <QFileDialog>
#include <QHBoxLayout>
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
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(8);

    auto* headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(0, 0, 0, 0);

    partyLabel_ = new QLabel(tr("Партия № —"), this);
    partyLabel_->setStyleSheet("font-weight: 600;");

    statusLabel_ = new QLabel(tr("Не сохранено"), this);
    statusLabel_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    statusLabel_->setStyleSheet("padding: 4px 8px; border-radius: 8px; background: #fee2e2; color: #991b1b; font-weight: 600;");

    headerLayout->addWidget(partyLabel_);
    headerLayout->addStretch();
    headerLayout->addWidget(statusLabel_);

    saveButton_ = new QPushButton(tr("Сохранить партию"), this);

    auto* secondaryLayout = new QHBoxLayout();
    secondaryLayout->setContentsMargins(0, 0, 0, 0);
    secondaryLayout->setSpacing(6);

    showInExplorerButton_ = new QPushButton(tr("Показать в проводнике"), this);
    saveAsButton_ = new QPushButton(tr("Сохранить как"), this);
    showInExplorerButton_->setProperty("variant", "secondary");
    saveAsButton_->setProperty("variant", "secondary");

    secondaryLayout->addWidget(showInExplorerButton_);
    secondaryLayout->addWidget(saveAsButton_);
    secondaryLayout->addStretch();

    errorLabel_ = new QLabel(this);
    errorLabel_->setWordWrap(true);
    errorLabel_->setStyleSheet("color: #b91c1c;");
    errorLabel_->hide();

    root->addLayout(headerLayout);
    root->addWidget(saveButton_);
    root->addLayout(secondaryLayout);
    root->addWidget(errorLabel_);
}

void QtCalibrationResultSaveWidget::bind()
{
    connect(saveButton_, &QPushButton::clicked, this, [this] { vm_.save(); });
    connect(showInExplorerButton_, &QPushButton::clicked, this, [this] { vm_.showInExplorer(); });
    connect(saveAsButton_, &QPushButton::clicked, this, [this] {
        const auto dir = QFileDialog::getExistingDirectory(this, tr("Выберите папку для сохранения"));
        if (!dir.isEmpty()) {
            vm_.saveAs(dir.toStdString());
        }
    });

    partySub_ = vm_.party_id.subscribe([this](const auto& change) {
        QMetaObject::invokeMethod(this, [this, party_id = change.new_value]() {
            partyLabel_->setText(party_id > 0 ? tr("Партия № %1").arg(party_id) : tr("Партия № —"));
        }, Qt::QueuedConnection);
    }, false);

    statusSub_ = vm_.status_text.subscribe([this](const auto& change) {
        QMetaObject::invokeMethod(this, [this, text = QString::fromStdString(change.new_value)]() {
            statusLabel_->setText(text);
        }, Qt::QueuedConnection);
    }, false);

    stateSub_ = vm_.save_state.subscribe([this](const auto& change) {
        QMetaObject::invokeMethod(this, [this, state = change.new_value]() {
            applyState(state);
        }, Qt::QueuedConnection);
    }, false);

    errorSub_ = vm_.error_text.subscribe([this](const auto& change) {
        QMetaObject::invokeMethod(this, [this, text = QString::fromStdString(change.new_value)]() {
            errorLabel_->setVisible(!text.isEmpty());
            errorLabel_->setText(text);
        }, Qt::QueuedConnection);
    }, false);

    canSaveSub_ = vm_.can_save.subscribe([this](const auto& change) {
        QMetaObject::invokeMethod(this, [this, value = change.new_value]() { saveButton_->setEnabled(value); }, Qt::QueuedConnection);
    }, false);
    canShowSub_ = vm_.can_show_in_explorer.subscribe([this](const auto& change) {
        QMetaObject::invokeMethod(this, [this, value = change.new_value]() { showInExplorerButton_->setEnabled(value); }, Qt::QueuedConnection);
    }, false);
    canSaveAsSub_ = vm_.can_save_as.subscribe([this](const auto& change) {
        QMetaObject::invokeMethod(this, [this, value = change.new_value]() { saveAsButton_->setEnabled(value); }, Qt::QueuedConnection);
    }, false);

    partyLabel_->setText(vm_.party_id.get_copy() > 0 ? tr("Партия № %1").arg(vm_.party_id.get_copy()) : tr("Партия № —"));
    statusLabel_->setText(QString::fromStdString(vm_.status_text.get_copy()));
    errorLabel_->setText(QString::fromStdString(vm_.error_text.get_copy()));
    errorLabel_->setVisible(!vm_.error_text.get_copy().empty());
    saveButton_->setEnabled(vm_.can_save.get_copy());
    showInExplorerButton_->setEnabled(vm_.can_show_in_explorer.get_copy());
    saveAsButton_->setEnabled(vm_.can_save_as.get_copy());
    applyState(vm_.save_state.get_copy());
}

void QtCalibrationResultSaveWidget::applyState(mvvm::CalibrationResultSaveViewModel::SaveState state)
{
    switch (state) {
    case mvvm::CalibrationResultSaveViewModel::SaveState::Saved:
        statusLabel_->setStyleSheet("padding: 4px 8px; border-radius: 8px; background: #dcfce7; color: #166534; font-weight: 600;");
        break;
    case mvvm::CalibrationResultSaveViewModel::SaveState::Saving:
        statusLabel_->setStyleSheet("padding: 4px 8px; border-radius: 8px; background: #dbeafe; color: #1d4ed8; font-weight: 600;");
        break;
    case mvvm::CalibrationResultSaveViewModel::SaveState::Error:
        statusLabel_->setStyleSheet("padding: 4px 8px; border-radius: 8px; background: #fee2e2; color: #991b1b; font-weight: 600;");
        break;
    case mvvm::CalibrationResultSaveViewModel::SaveState::NotSaved:
    default:
        statusLabel_->setStyleSheet("padding: 4px 8px; border-radius: 8px; background: #f3f4f6; color: #374151; font-weight: 600;");
        break;
    }
}

} // namespace ui
