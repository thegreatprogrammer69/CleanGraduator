#include "QtCalibrationResultSaveWidget.h"

#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
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
        const auto selected_cameras = requestSelectedCameras();
        if (!selected_cameras.has_value()) {
            return;
        }

        const auto result = vm_.saveForCameras(*selected_cameras);
        showSaveFeedbackMessage(result);
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

std::optional<std::vector<int>> QtCalibrationResultSaveWidget::requestSelectedCameras()
{
    const auto cameras = vm_.cameraNumbers();
    if (cameras.empty()) {
        QMessageBox::warning(
            this,
            tr("Сохранение результата"),
            tr("Нет доступных камер для сохранения."));
        return std::nullopt;
    }

    QDialog dialog(this);
    dialog.setWindowTitle(tr("Выбор камер для сохранения"));
    dialog.setModal(true);

    auto* layout = new QVBoxLayout(&dialog);
    auto* table = new QTableWidget(2, static_cast<int>(cameras.size()), &dialog);
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    for (int col = 0; col < static_cast<int>(cameras.size()); ++col) {
        auto* camera_item = new QTableWidgetItem(QString::number(cameras[col]));
        camera_item->setTextAlignment(Qt::AlignCenter);
        table->setItem(0, col, camera_item);

        auto* check_item = new QTableWidgetItem();
        check_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
        check_item->setCheckState(Qt::Checked);
        table->setItem(1, col, check_item);
    }

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialog);
    buttons->button(QDialogButtonBox::Save)->setText(tr("Сохранить"));
    buttons->button(QDialogButtonBox::Cancel)->setText(tr("Отмена"));

    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    layout->addWidget(table);
    layout->addWidget(buttons);

    if (dialog.exec() != QDialog::Accepted) {
        return std::nullopt;
    }

    std::vector<int> selected;
    for (int col = 0; col < table->columnCount(); ++col) {
        const auto* item = table->item(1, col);
        if (item != nullptr && item->checkState() == Qt::Checked) {
            selected.push_back(cameras[col]);
        }
    }

    return selected;
}

void QtCalibrationResultSaveWidget::showSaveFeedbackMessage(
    const mvvm::CalibrationResultSaveViewModel::SaveActionFeedback& result)
{
    QMessageBox message_box(this);
    message_box.setWindowTitle(tr("Сохранение результата"));

    QString text;
    if (result.success) {
        text = tr("Результат успешно сохранён.\nПуть: %1")
                   .arg(QString::fromStdString(result.saved_to.string()));
        message_box.setIcon(QMessageBox::Information);
        message_box.setText(text);

        const auto open_button = message_box.addButton(tr("Показать в проводнике"), QMessageBox::ActionRole);
        message_box.addButton(QMessageBox::Ok);
        message_box.exec();

        if (message_box.clickedButton() == open_button) {
            openInExplorer(result.saved_to);
        }
        return;
    }

    const QString path_text = result.saved_to.empty()
        ? tr("не определён")
        : QString::fromStdString(result.saved_to.string());
    text = tr("Ошибка сохранения: %1\nПуть: %2")
               .arg(QString::fromStdString(result.error), path_text);
    message_box.setIcon(QMessageBox::Critical);
    message_box.setText(text);
    message_box.exec();
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
