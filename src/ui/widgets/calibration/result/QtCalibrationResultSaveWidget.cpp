#include "QtCalibrationResultSaveWidget.h"

#include <QDesktopServices>
#include <QDialog>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QMetaObject>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QTableWidget>
#include <QUrl>
#include <QVBoxLayout>
#include <QVariant>

namespace ui {

    namespace {

        std::filesystem::path qStringToPath(const QString& value)
        {
#ifdef _WIN32
            return std::filesystem::path(value.toStdWString());
#else
            return std::filesystem::path(value.toUtf8().constData());
#endif
        }

        QString pathToQString(const std::filesystem::path& path)
        {
#ifdef _WIN32
            return QString::fromStdWString(path.native());
#else
            return QString::fromUtf8(path.native().c_str());
#endif
        }

    } // namespace

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

    saveAllButton_ = new QPushButton(tr("Сохранить всё"), this);

    auto* mainButtonLayout = new QHBoxLayout();
    mainButtonLayout->setSpacing(8);
    mainButtonLayout->addWidget(saveAllButton_);
    mainButtonLayout->addStretch(1);

    auto* secondaryButtonLayout = new QHBoxLayout();
    secondaryButtonLayout->setSpacing(8);

    saveSelectedButton_ = new QPushButton(tr("Сохранить только выбранные"), this);
    showInExplorerButton_ = new QPushButton(tr("Показать в проводнике"), this);

    showInExplorerButton_->setProperty("type", "secondary");

    secondaryButtonLayout->addWidget(saveSelectedButton_);
    secondaryButtonLayout->addWidget(showInExplorerButton_);
    secondaryButtonLayout->addStretch(1);

    errorLabel_ = new QLabel(this);
    errorLabel_->setWordWrap(true);
    errorLabel_->setStyleSheet("color: #dc2626;");

    rootLayout->addLayout(topLayout);
    rootLayout->addLayout(mainButtonLayout);
    rootLayout->addLayout(secondaryButtonLayout);
    rootLayout->addWidget(errorLabel_);
}

void QtCalibrationResultSaveWidget::bind()
{
    connect(saveAllButton_, &QPushButton::clicked, this, [this] {
        const auto result = vm_.saveWithoutErrors();
        showSaveResultMessage(result);
    });

    connect(saveSelectedButton_, &QPushButton::clicked, this, [this] {
        const auto selection = requestCameraSelection();
        if (!selection.accepted) {
            return;
        }
        if (selection.selected_camera_ids.empty()) {
            QMessageBox::warning(
                this,
                tr("Сохранение результата"),
                tr("Не выбрано ни одной камеры. Сохранение не выполнено."));
            return;
        }

        const auto result = vm_.save(selection.selected_camera_ids);
        showSaveResultMessage(result);
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
            saveAllButton_->setEnabled(enabled);
            saveSelectedButton_->setEnabled(enabled);
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
    saveAllButton_->setEnabled(vm_.can_save.get_copy());
    saveSelectedButton_->setEnabled(vm_.can_save.get_copy());
    showInExplorerButton_->setEnabled(vm_.can_show_in_explorer.get_copy());
}

void QtCalibrationResultSaveWidget::updateStateBadge(mvvm::CalibrationResultSaveState state, const QString& text)
{
    QString color = "#64748b";
    QString fg = "#ffffff";

    switch (state) {
    case mvvm::CalibrationResultSaveState::Saved:
        color = "#0f766e";
        break;
    case mvvm::CalibrationResultSaveState::Saving:
        color = "#b45309";
        break;
    case mvvm::CalibrationResultSaveState::Error:
        color = "#dc2626";
        break;
    case mvvm::CalibrationResultSaveState::NotSaved:
    default:
        color = "#64748b";
        break;
    }

    stateLabel_->setText(text);
    stateLabel_->setStyleSheet(QString(
        "background:%1; color:%2; border-radius:10px; padding:4px 10px; font-weight:600;")
        .arg(color, fg));
}

QtCalibrationResultSaveWidget::CameraSelectionResult QtCalibrationResultSaveWidget::requestCameraSelection() const
{
    CameraSelectionResult result{};
    const auto camera_ids = vm_.availableCameraIds();
    if (camera_ids.empty()) {
        return result;
    }

    QDialog dialog(const_cast<QtCalibrationResultSaveWidget*>(this));
    dialog.setWindowTitle(tr("Выбор камер для сохранения"));
    dialog.setModal(true);

    auto* root = new QVBoxLayout(&dialog);
    auto* table = new QTableWidget(2, static_cast<int>(camera_ids.size()), &dialog);
    table->horizontalHeader()->setVisible(false);
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setFocusPolicy(Qt::NoFocus);
    table->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->setMinimumWidth(420);

    for (int col = 0; col < static_cast<int>(camera_ids.size()); ++col) {
        const int camera_id = camera_ids[static_cast<std::size_t>(col)];
        table->setItem(0, col, new QTableWidgetItem(QString::number(camera_id)));

        auto* checkbox = new QCheckBox(&dialog);
        checkbox->setChecked(true);
        checkbox->setProperty("camera_id", camera_id);
        checkbox->setStyleSheet("margin-left:12px;");
        table->setCellWidget(1, col, checkbox);
    }

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Save | QDialogButtonBox::Cancel,
        Qt::Horizontal,
        &dialog);
    buttons->button(QDialogButtonBox::Save)->setText(tr("Сохранить"));
    buttons->button(QDialogButtonBox::Cancel)->setText(tr("Отмена"));

    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    root->addWidget(table);
    root->addWidget(buttons);

    if (dialog.exec() != QDialog::Accepted) {
        return result;
    }

    result.accepted = true;
    for (int col = 0; col < table->columnCount(); ++col) {
        auto* checkbox = qobject_cast<QCheckBox*>(table->cellWidget(1, col));
        if (checkbox != nullptr && checkbox->isChecked()) {
            result.selected_camera_ids.push_back(checkbox->property("camera_id").toInt());
        }
    }

    return result;
}

void QtCalibrationResultSaveWidget::showSaveResultMessage(
    const application::usecase::SaveCalibrationResult::Result& result)
{
    if (result.success) {
        QMessageBox msg(this);
        msg.setIcon(QMessageBox::Information);
        msg.setWindowTitle(tr("Сохранение результата"));
        msg.setText(tr("Результат успешно сохранён."));
        msg.setInformativeText(tr("Путь: %1").arg(pathToQString(result.saved_to)));
        auto* openButton = msg.addButton(tr("Показать в проводнике"), QMessageBox::ActionRole);
        msg.addButton(QMessageBox::Ok);
        msg.exec();

        if (msg.clickedButton() == openButton) {
            openInExplorer(result.saved_to);
        }
        return;
    }

    QMessageBox::critical(
        this,
        tr("Сохранение результата"),
        tr("Не удалось сохранить результат.\n%1\nПуть: %2")
            .arg(QString::fromStdString(result.error),
                pathToQString(result.saved_to)));
}

void QtCalibrationResultSaveWidget::openInExplorer(const std::filesystem::path& path)
{
    if (path.empty()) {
        return;
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(pathToQString(path)));
}

} // namespace ui
