#include "QtDualValveControlWidget.h"

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

#include "viewmodels/control/DualValveControlViewModel.h"
#include "viewmodels/Observable.h"

namespace ui {

static const char* toText(mvvm::DualValveControlViewModel::FlapsState s) {
    using S = mvvm::DualValveControlViewModel::FlapsState;
    switch (s) {
        case S::FlapsUninitialized: return "Неизвестно";
        case S::InputFlapOpened:    return "Впускной открыт";
        case S::OutputFlapOpened:   return "Выпускной открыт";
        case S::FlapsClosed:        return "Клапана закрыты";
        default:                    return "Неизвестно";
    }
}

QtDualValveControlWidget::QtDualValveControlWidget(mvvm::DualValveControlViewModel& vm, QWidget* parent)
    : QWidget(parent)
    , vm_(vm)
{
    setupUi();
    setupConnections();
    bindViewModel();
}

QtDualValveControlWidget::~QtDualValveControlWidget() = default;

void QtDualValveControlWidget::setupUi()
{
    open_input_btn_  = new QPushButton(tr("Открыть впускной"), this);
    open_output_btn_ = new QPushButton(tr("Открыть выпускной"), this);
    close_flaps_btn_ = new QPushButton(tr("Закрыть клапана"), this);
    state_label_     = new QLabel(tr("Неизвестно"), this);

    // 2 строки кнопок:
    // 1 строка: Открыть впускной | Открыть выпускной
    // 2 строка: Закрыть клапана (на две колонки)
    auto* grid = new QGridLayout(this);
    grid->addWidget(open_input_btn_,  0, 0);
    grid->addWidget(open_output_btn_, 0, 1);
    grid->addWidget(close_flaps_btn_, 1, 0, 1, 2);

    // (опционально) строка статуса снизу — без стилей
    grid->addWidget(state_label_, 2, 0, 1, 2);

    setLayout(grid);
}

void QtDualValveControlWidget::setupConnections()
{
    connect(open_input_btn_, &QPushButton::clicked, this, [this] {
        vm_.openInputFlap();
    });

    connect(open_output_btn_, &QPushButton::clicked, this, [this] {
        vm_.openOutputFlap();
    });

    connect(close_flaps_btn_, &QPushButton::clicked, this, [this] {
        vm_.closeFlaps();
    });
}

void QtDualValveControlWidget::bindViewModel()
{
    // первичная отрисовка
    updateUiFromState(vm_.flaps_state.get_copy());

    // реакция на изменения (предполагаю, что Observable вызывает колбэк из любого потока —
    // тогда через invokeMethod в UI-поток)
    state_sub_ = vm_.flaps_state.subscribe([this](const auto& state) {
        QMetaObject::invokeMethod(this, [this, state] {
            updateUiFromState(state.new_value);
        }, Qt::QueuedConnection);
    });
}

void QtDualValveControlWidget::updateUiFromState(mvvm::DualValveControlViewModel::FlapsState state)
{
    state_label_->setText(QString::fromUtf8(toText(state)));

    // базовая логика доступности кнопок (можешь выкинуть, если не нужно)
    using S = mvvm::DualValveControlViewModel::FlapsState;

    const bool is_initialized = (state != S::FlapsUninitialized);

    open_input_btn_->setEnabled(true);
    open_output_btn_->setEnabled(true);
    close_flaps_btn_->setEnabled(is_initialized);

    if (state == S::InputFlapOpened) {
        open_input_btn_->setEnabled(false);
    } else if (state == S::OutputFlapOpened) {
        open_output_btn_->setEnabled(false);
    } else if (state == S::FlapsClosed) {
        close_flaps_btn_->setEnabled(false);
    }
}

} // namespace ui
