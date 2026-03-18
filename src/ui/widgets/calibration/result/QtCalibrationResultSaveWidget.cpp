#include "QtCalibrationResultSaveWidget.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QUrl>
#include <QVBoxLayout>

namespace ui {

struct QtCalibrationResultSaveWidget::Subscriptions {
    mvvm::Observable<std::string>::Subscription party_title;
    mvvm::Observable<std::string>::Subscription status_text;
    mvvm::Observable<std::string>::Subscription status_color;
    mvvm::Observable<std::string>::Subscription error_text;
    mvvm::Observable<bool>::Subscription can_save;
    mvvm::Observable<bool>::Subscription can_save_as;
    mvvm::Observable<bool>::Subscription can_show_in_explorer;
};

QtCalibrationResultSaveWidget::QtCalibrationResultSaveWidget(
    mvvm::CalibrationResultSaveViewModel& vm,
    QWidget* parent)
    : QWidget(parent)
    , vm_(vm)
    , subscriptions_(std::make_unique<Subscriptions>())
{
    setupUi();
    bindViewModel();
}

void QtCalibrationResultSaveWidget::setupUi()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(8);

    auto* top_row = new QHBoxLayout();
    top_row->setContentsMargins(0, 0, 0, 0);
    top_row->setSpacing(10);

    party_label_ = new QLabel(this);
    status_label_ = new QLabel(this);
    status_label_->setAlignment(Qt::AlignCenter);
    status_label_->setMinimumWidth(120);

    top_row->addWidget(party_label_, 0, Qt::AlignVCenter);
    top_row->addStretch(1);
    top_row->addWidget(status_label_, 0, Qt::AlignVCenter);

    save_button_ = new QPushButton(tr("Сохранить партию"), this);
    save_button_->setProperty("variant", "primary");

    auto* secondary_row = new QHBoxLayout();
    secondary_row->setContentsMargins(0, 0, 0, 0);
    secondary_row->setSpacing(8);

    show_in_explorer_button_ = new QPushButton(tr("Показать в проводнике"), this);
    show_in_explorer_button_->setProperty("variant", "secondary");
    show_in_explorer_button_->setProperty("size", "small");

    save_as_button_ = new QPushButton(tr("Сохранить как"), this);
    save_as_button_->setProperty("variant", "secondary");
    save_as_button_->setProperty("size", "small");

    secondary_row->addWidget(show_in_explorer_button_);
    secondary_row->addWidget(save_as_button_);
    secondary_row->addStretch(1);

    error_label_ = new QLabel(this);
    error_label_->setWordWrap(true);
    error_label_->setStyleSheet("color: #b91c1c;");
    error_label_->hide();

    root->addLayout(top_row);
    root->addWidget(save_button_, 0, Qt::AlignLeft);
    root->addLayout(secondary_row);
    root->addWidget(error_label_);

    connect(save_button_, &QPushButton::clicked, this, [this] { vm_.save(); });
    connect(save_as_button_, &QPushButton::clicked, this, &QtCalibrationResultSaveWidget::chooseSaveAsDirectory);
    connect(show_in_explorer_button_, &QPushButton::clicked, this, &QtCalibrationResultSaveWidget::openInExplorer);
}

void QtCalibrationResultSaveWidget::bindViewModel()
{
    subscriptions_->party_title = vm_.party_title.subscribe([this](const auto& change) {
        party_label_->setText(QString::fromStdString(change.new_value));
    }, false);

    subscriptions_->status_text = vm_.status_text.subscribe([this](const auto& change) {
        status_label_->setText(QString::fromStdString(change.new_value));
    }, false);

    subscriptions_->status_color = vm_.status_color.subscribe([this](const auto& change) {
        status_label_->setStyleSheet(QString(
            "background-color: %1; color: white; border-radius: 10px; padding: 4px 10px; font-weight: 600;")
            .arg(QString::fromStdString(change.new_value)));
    }, false);

    subscriptions_->error_text = vm_.error_text.subscribe([this](const auto& change) {
        const auto text = QString::fromStdString(change.new_value);
        error_label_->setText(text);
        error_label_->setVisible(!text.isEmpty());
    }, false);

    subscriptions_->can_save = vm_.can_save.subscribe([this](const auto& change) {
        save_button_->setEnabled(change.new_value);
    }, false);

    subscriptions_->can_save_as = vm_.can_save_as.subscribe([this](const auto& change) {
        save_as_button_->setEnabled(change.new_value);
    }, false);

    subscriptions_->can_show_in_explorer = vm_.can_show_in_explorer.subscribe([this](const auto& change) {
        show_in_explorer_button_->setEnabled(change.new_value);
    }, false);

    party_label_->setText(QString::fromStdString(vm_.party_title.get_copy()));
    status_label_->setText(QString::fromStdString(vm_.status_text.get_copy()));
    status_label_->setStyleSheet(QString(
        "background-color: %1; color: white; border-radius: 10px; padding: 4px 10px; font-weight: 600;")
        .arg(QString::fromStdString(vm_.status_color.get_copy())));
    const auto error = QString::fromStdString(vm_.error_text.get_copy());
    error_label_->setText(error);
    error_label_->setVisible(!error.isEmpty());
    save_button_->setEnabled(vm_.can_save.get_copy());
    save_as_button_->setEnabled(vm_.can_save_as.get_copy());
    show_in_explorer_button_->setEnabled(vm_.can_show_in_explorer.get_copy());
}

void QtCalibrationResultSaveWidget::chooseSaveAsDirectory()
{
    const auto directory = QFileDialog::getExistingDirectory(
        this,
        tr("Выберите каталог для сохранения результата"));

    if (!directory.isEmpty()) {
        vm_.saveAs(directory.toStdString());
    }
}

void QtCalibrationResultSaveWidget::openInExplorer()
{
    const auto last_directory = vm_.last_saved_directory.get_copy();
    if (!last_directory) {
        return;
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(last_directory->string())));
}

} // namespace ui
