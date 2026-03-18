#include "QtCalibrationResultSaveWidget.h"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace ui {

QtCalibrationResultSaveWidget::QtCalibrationResultSaveWidget(
    mvvm::CalibrationResultSaveViewModel& vm,
    QWidget* parent)
    : QWidget(parent)
    , vm_(vm)
{
    setupUi();
    bindViewModel();
    vm_.refresh();
}

void QtCalibrationResultSaveWidget::setupUi() {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(8);

    auto* top = new QHBoxLayout();
    top->setSpacing(8);

    batch_label_ = new QLabel(tr("Партия № —"), this);
    batch_label_->setProperty("variant", "big");

    status_badge_ = new QLabel(tr("Не сохранено"), this);
    status_badge_->setAlignment(Qt::AlignCenter);
    status_badge_->setMinimumWidth(120);
    status_badge_->setStyleSheet("padding: 4px 10px; border-radius: 10px; background: #eef0f3; color: #4b5563;");

    top->addWidget(batch_label_);
    top->addWidget(status_badge_, 0, Qt::AlignLeft);
    top->addStretch(1);

    save_button_ = new QPushButton(tr("Сохранить партию"), this);
    save_button_->setProperty("variant", "primary");

    reveal_button_ = new QPushButton(tr("Показать в проводнике"), this);
    reveal_button_->setProperty("variant", "secondary");

    save_as_button_ = new QPushButton(tr("Сохранить как"), this);
    save_as_button_->setProperty("variant", "secondary");

    auto* buttons = new QHBoxLayout();
    buttons->setSpacing(8);
    buttons->addWidget(save_button_);
    buttons->addWidget(reveal_button_);
    buttons->addWidget(save_as_button_);
    buttons->addStretch(1);

    error_label_ = new QLabel(this);
    error_label_->setWordWrap(true);
    error_label_->setStyleSheet("color: #b42318;");
    error_label_->hide();

    root->addLayout(top);
    root->addLayout(buttons);
    root->addWidget(error_label_);

    connect(save_button_, &QPushButton::clicked, this, [this] {
        vm_.save();
    });
    connect(reveal_button_, &QPushButton::clicked, this, [this] {
        vm_.revealInExplorer();
    });
    connect(save_as_button_, &QPushButton::clicked, this, [this] {
        const auto directory = QFileDialog::getExistingDirectory(
            this,
            tr("Выберите папку для сохранения результата"));
        if (!directory.isEmpty()) {
            vm_.saveAs(directory.toStdString());
        }
    });

    polishButtonStyles();
}

void QtCalibrationResultSaveWidget::bindViewModel() {
    batch_number_sub_ = vm_.batch_number.subscribe([this](const auto& ev) {
        if (ev.new_value.has_value()) {
            batch_label_->setText(tr("Партия № %1").arg(*ev.new_value));
        } else {
            batch_label_->setText(tr("Партия № —"));
        }
    });

    auto applyStatusBadge = [this] {
        applyStatus(vm_.status.get_copy(), QString::fromStdString(vm_.status_text.get_copy()));
    };

    status_sub_ = vm_.status.subscribe([applyStatusBadge](const auto&) {
        applyStatusBadge();
    });
    status_text_sub_ = vm_.status_text.subscribe([applyStatusBadge](const auto&) {
        applyStatusBadge();
    });

    error_text_sub_ = vm_.error_text.subscribe([this](const auto& ev) {
        const auto text = QString::fromStdString(ev.new_value);
        error_label_->setVisible(!text.isEmpty());
        error_label_->setText(text);
    });

    can_save_sub_ = vm_.can_save.subscribe([this](const auto& ev) {
        save_button_->setEnabled(ev.new_value);
        save_as_button_->setEnabled(ev.new_value);
    });

    can_reveal_sub_ = vm_.can_reveal.subscribe([this](const auto& ev) {
        reveal_button_->setEnabled(ev.new_value);
    });
}

void QtCalibrationResultSaveWidget::polishButtonStyles() {
    for (auto* button : {save_button_, reveal_button_, save_as_button_}) {
        button->style()->unpolish(button);
        button->style()->polish(button);
        button->update();
    }
}

void QtCalibrationResultSaveWidget::applyStatus(
    mvvm::CalibrationResultSaveViewModel::Status status,
    const QString& text) {
    QString style;

    switch (status) {
        case mvvm::CalibrationResultSaveViewModel::Status::Unsaved:
            style = "padding: 4px 10px; border-radius: 10px; background: #eef0f3; color: #4b5563;";
            break;
        case mvvm::CalibrationResultSaveViewModel::Status::Saving:
            style = "padding: 4px 10px; border-radius: 10px; background: #fff4ce; color: #9a6700;";
            break;
        case mvvm::CalibrationResultSaveViewModel::Status::Saved:
            style = "padding: 4px 10px; border-radius: 10px; background: #dcfce7; color: #166534;";
            break;
        case mvvm::CalibrationResultSaveViewModel::Status::Error:
            style = "padding: 4px 10px; border-radius: 10px; background: #fee2e2; color: #b91c1c;";
            break;
    }

    status_badge_->setText(text);
    status_badge_->setStyleSheet(style);
}

}
