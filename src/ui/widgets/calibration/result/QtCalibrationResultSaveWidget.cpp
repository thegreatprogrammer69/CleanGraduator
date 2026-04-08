#include "QtCalibrationResultSaveWidget.h"

#include <QDesktopServices>
#include <QDialog>
#include <QFileDialog>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QMetaObject>
#include <QTableWidget>
#include <QTableWidgetItem>
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

void QtCalibrationResultSaveWidget::setupUi() {
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

    saveButton_ = new QPushButton(tr("Сохранить партию"), this);

    auto* mainButtonLayout = new QHBoxLayout();
    mainButtonLayout->setSpacing(8);
    mainButtonLayout->addWidget(saveButton_);
    mainButtonLayout->addStretch(1);

    auto* secondaryButtonLayout = new QHBoxLayout();
    secondaryButtonLayout->setSpacing(8);

    showInExplorerButton_ = new QPushButton(tr("Показать в проводнике"), this);
    saveAsButton_ = new QPushButton(tr("Сохранить как"), this);

    showInExplorerButton_->setProperty("type", "secondary");
    saveAsButton_->setProperty("type", "secondary");

    secondaryButtonLayout->addWidget(showInExplorerButton_);
    secondaryButtonLayout->addWidget(saveAsButton_);
    secondaryButtonLayout->addStretch(1);

    errorLabel_ = new QLabel(this);
    errorLabel_->setWordWrap(true);
    errorLabel_->setStyleSheet("color: #b91c1c;");

    rootLayout->addLayout(topLayout);
    rootLayout->addLayout(mainButtonLayout);
    rootLayout->addLayout(secondaryButtonLayout);
    rootLayout->addWidget(errorLabel_);
}

void QtCalibrationResultSaveWidget::bind()
{
    connect(saveButton_, &QPushButton::clicked, this, [this] {
        const auto selected_camera_ids = requestCameraSelection();
        if (!selected_camera_ids.has_value()) {
            return;
        }

        const auto result = vm_.save(*selected_camera_ids);
        showSaveResult(result);
    });

    connect(saveAsButton_, &QPushButton::clicked, this, [this] {
        const QString directory = QFileDialog::getExistingDirectory(
            this,
            tr("Выберите директорию для сохранения"));

        if (directory.isEmpty()) {
            return;
        }

        const auto selected_camera_ids = requestCameraSelection();
        if (!selected_camera_ids.has_value()) {
            return;
        }

        const auto result = vm_.saveAs(directory.toStdString(), *selected_camera_ids);
        showSaveResult(result);
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

std::optional<std::vector<int>> QtCalibrationResultSaveWidget::requestCameraSelection()
{
    const auto camera_ids = vm_.available_camera_ids.get_copy();
    if (camera_ids.empty()) {
        QMessageBox::warning(
            this,
            tr("Сохранение результата"),
            tr("Нет доступных камер для сохранения результата."));
        return std::nullopt;
    }

    QDialog dialog(this);
    dialog.setWindowTitle(tr("Выбор камер для сохранения"));
    dialog.resize(480, 180);

    auto* layout = new QVBoxLayout(&dialog);

    auto* table = new QTableWidget(2, static_cast<int>(camera_ids.size()), &dialog);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->verticalHeader()->setVisible(true);
    table->horizontalHeader()->setVisible(false);
    table->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setVerticalHeaderItem(0, new QTableWidgetItem(tr("Камера")));
    table->setVerticalHeaderItem(1, new QTableWidgetItem(tr("Сохранить")));

    std::vector<QCheckBox*> checkboxes;
    checkboxes.reserve(camera_ids.size());

    for (int col = 0; col < static_cast<int>(camera_ids.size()); ++col) {
        auto* camera_item = new QTableWidgetItem(QString::number(camera_ids[static_cast<size_t>(col)));
        camera_item->setTextAlignment(Qt::AlignCenter);
        table->setItem(0, col, camera_item);

        auto* checkbox = new QCheckBox(&dialog);
        checkbox->setChecked(true);
        auto* wrapper = new QWidget(&dialog);
        auto* wrapper_layout = new QHBoxLayout(wrapper);
        wrapper_layout->setContentsMargins(0, 0, 0, 0);
        wrapper_layout->addStretch();
        wrapper_layout->addWidget(checkbox);
        wrapper_layout->addStretch();
        table->setCellWidget(1, col, wrapper);
        checkboxes.push_back(checkbox);
    }

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialog);
    buttons->button(QDialogButtonBox::Save)->setText(tr("Сохранить"));
    buttons->button(QDialogButtonBox::Cancel)->setText(tr("Отмена"));

    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    layout->addWidget(table);
    layout->addWidget(buttons);

    if (dialog.exec() != QDialog::Accepted) {
        return std::nullopt;
    }

    std::vector<int> selected_camera_ids;
    for (size_t i = 0; i < camera_ids.size(); ++i) {
        if (checkboxes[i]->isChecked()) {
            selected_camera_ids.push_back(camera_ids[i]);
        }
    }

    return selected_camera_ids;
}

void QtCalibrationResultSaveWidget::showSaveResult(const application::usecase::SaveCalibrationResult::Result& result)
{
    const QString path = result.saved_to.empty()
        ? tr("не определён")
        : QString::fromStdString(result.saved_to.string());

    if (!result.success) {
        QMessageBox::critical(
            this,
            tr("Сохранение результата"),
            tr("Сохранение завершилось с ошибкой.\nПуть: %1\nОшибка: %2")
                .arg(path, QString::fromStdString(result.error)));
        return;
    }

    QMessageBox box(this);
    box.setIcon(QMessageBox::Information);
    box.setWindowTitle(tr("Сохранение результата"));
    box.setText(tr("Результат успешно сохранён.\nПуть: %1").arg(path));
    box.addButton(tr("ОК"), QMessageBox::AcceptRole);
    auto* show_button = box.addButton(tr("Показать в проводнике"), QMessageBox::ActionRole);

    box.exec();
    if (box.clickedButton() == show_button) {
        openInExplorer(result.saved_to);
    }
}

} // namespace ui
