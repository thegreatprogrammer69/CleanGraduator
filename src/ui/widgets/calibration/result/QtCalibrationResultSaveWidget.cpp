#include "QtCalibrationResultSaveWidget.h"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace ui {

QtCalibrationResultSaveWidget::QtCalibrationResultSaveWidget(mvvm::CalibrationResultSaveViewModel& vm,
                                                             QWidget* parent)
    : QWidget(parent), vm_(vm)
{
    buildUi();
    connectUi();
    connectViewModel();
    applyState(vm_.state.get_copy());
}

QtCalibrationResultSaveWidget::~QtCalibrationResultSaveWidget() = default;

void QtCalibrationResultSaveWidget::buildUi() {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(8);

    auto* top = new QHBoxLayout();
    top->setSpacing(12);

    party_label_ = new QLabel(this);
    party_label_->setProperty("variant", "big");

    status_label_ = new QLabel(this);
    status_label_->setAlignment(Qt::AlignCenter);
    status_label_->setMinimumWidth(120);
    status_label_->setStyleSheet("padding: 4px 10px; border-radius: 10px; font-weight: 600;");

    top->addWidget(party_label_);
    top->addWidget(status_label_, 0, Qt::AlignLeft);
    top->addStretch(1);

    save_button_ = new QPushButton(tr("Сохранить партию"), this);
    save_button_->setProperty("variant", "primary");
    save_button_->setMinimumWidth(180);

    auto* secondary = new QHBoxLayout();
    secondary->setSpacing(8);

    show_in_explorer_button_ = new QPushButton(tr("Показать в проводнике"), this);
    show_in_explorer_button_->setProperty("variant", "secondary");
    show_in_explorer_button_->setProperty("size", "small");

    save_as_button_ = new QPushButton(tr("Сохранить как"), this);
    save_as_button_->setProperty("variant", "secondary");
    save_as_button_->setProperty("size", "small");

    secondary->addWidget(show_in_explorer_button_, 0, Qt::AlignLeft);
    secondary->addWidget(save_as_button_, 0, Qt::AlignLeft);
    secondary->addStretch(1);

    path_label_ = new QLabel(this);
    path_label_->setWordWrap(true);
    path_label_->setTextInteractionFlags(Qt::TextSelectableByMouse);

    error_label_ = new QLabel(this);
    error_label_->setWordWrap(true);
    error_label_->setStyleSheet("color: #dc2626;");

    root->addLayout(top);
    root->addWidget(save_button_, 0, Qt::AlignLeft);
    root->addLayout(secondary);
    root->addWidget(path_label_);
    root->addWidget(error_label_);
}

void QtCalibrationResultSaveWidget::connectUi() {
    connect(save_button_, &QPushButton::clicked, this, [this] { vm_.save(); });
    connect(show_in_explorer_button_, &QPushButton::clicked, this, [this] { vm_.showInExplorer(); });
    connect(save_as_button_, &QPushButton::clicked, this, [this] {
        const auto current_state = vm_.state.get_copy();
        const QString initial_dir = current_state.path.empty()
            ? QString{}
            : QString::fromStdString(current_state.path.string());

        const QString dir = QFileDialog::getExistingDirectory(
            this,
            tr("Выберите папку для сохранения"),
            initial_dir);

        if (!dir.isEmpty()) {
            vm_.saveAs(dir.toStdString());
        }
    });
}

void QtCalibrationResultSaveWidget::connectViewModel() {
    state_sub_ = vm_.state.subscribe([this](const auto& change) {
        applyState(change.new_value);
    }, false);
}

void QtCalibrationResultSaveWidget::applyState(const mvvm::CalibrationResultSaveViewModel::ViewState& state) {
    if (state.party_id > 0) {
        party_label_->setText(tr("Партия № %1").arg(state.party_id));
    } else {
        party_label_->setText(tr("Партия не назначена"));
    }

    status_label_->setText(QString::fromStdString(state.status_text));
    status_label_->setStyleSheet(QString("padding: 4px 10px; border-radius: 10px; font-weight: 600; color: white; background-color: %1;")
                                 .arg(QString::fromStdString(state.status_color)));

    if (state.path.empty()) {
        path_label_->setText(tr("Путь сохранения появится после выбора партии или успешного сохранения."));
    } else {
        path_label_->setText(tr("Папка: %1").arg(QString::fromStdString(state.path.string())));
    }

    error_label_->setVisible(!state.last_error.empty());
    error_label_->setText(QString::fromStdString(state.last_error));

    save_button_->setEnabled(state.can_save);
    show_in_explorer_button_->setEnabled(state.can_show_in_explorer);
    save_as_button_->setEnabled(state.can_save_as);
}

} // namespace ui
