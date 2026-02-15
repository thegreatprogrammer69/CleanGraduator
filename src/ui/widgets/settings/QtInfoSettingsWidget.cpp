#include "QtInfoSettingsWidget.h"

#include <QComboBox>
#include <QFormLayout>
#include <QLabel>

#include "viewmodels/settings/InfoSettingsViewModel.h"

QtInfoSettingsWidget::QtInfoSettingsWidget(
    mvvm::InfoSettingsViewModel& model,
    QWidget* parent
)
    : QWidget(parent)
    , model_(model)
{
    buildUi();
    connectUi();
    connectViewModel();
}

QtInfoSettingsWidget::~QtInfoSettingsWidget() = default;

void QtInfoSettingsWidget::buildUi() {
    auto* layout = new QFormLayout(this);

    displacementCombo_ = new QComboBox(this);
    gaugeCombo_ = new QComboBox(this);
    precisionCombo_ = new QComboBox(this);
    pressureUnitCombo_ = new QComboBox(this);
    printerCombo_ = new QComboBox(this);

    for (const auto& value : model_.displacements()) {
        displacementCombo_->addItem(QString::fromUtf8(value.c_str()));
    }

    for (const auto& value : model_.gauges()) {
        gaugeCombo_->addItem(QString::fromUtf8(value.c_str()));
    }

    for (const auto& value : model_.precisions()) {
        precisionCombo_->addItem(QString::fromUtf8(value.c_str()));
    }

    for (const auto& value : model_.pressureUnits()) {
        pressureUnitCombo_->addItem(QString::fromUtf8(value.c_str()));
    }

    for (const auto& value : model_.printers()) {
        printerCombo_->addItem(QString::fromUtf8(value.c_str()));
    }

    auto addRow = [&](const QString& text, QWidget* field)
    {
        auto* label = new QLabel(text, this);
        label->setProperty("variant", "big");
        layout->addRow(label, field);
    };

    addRow(tr("Манометр"), gaugeCombo_);
    addRow(tr("Ед. давления"), pressureUnitCombo_);
    addRow(tr("Класс точности"), precisionCombo_);
    addRow(tr("Подставка"), displacementCombo_);
    addRow(tr("Принтер"), printerCombo_);
}

void QtInfoSettingsWidget::connectUi() {
    connect(displacementCombo_, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int idx) {
        model_.selectedDisplacement.set(idx);
        model_.save();
    });

    connect(gaugeCombo_, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int idx) {
        model_.selectedGauge.set(idx);
        model_.save();
    });

    connect(precisionCombo_, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int idx) {
        model_.selectedPrecision.set(idx);
        model_.save();
    });

    connect(pressureUnitCombo_, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int idx) {
        model_.selectedPressureUnit.set(idx);
        model_.save();
    });

    connect(printerCombo_, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int idx) {
        model_.selectedPrinter.set(idx);
        model_.save();
    });
}

void QtInfoSettingsWidget::connectViewModel() {
    displacementSub_ = model_.selectedDisplacement.subscribe([this](const auto& ev) {
        if (displacementCombo_->currentIndex() != ev.new_value) {
            displacementCombo_->setCurrentIndex(ev.new_value);
        }
    }, false);

    gaugeSub_ = model_.selectedGauge.subscribe([this](const auto& ev) {
        if (gaugeCombo_->currentIndex() != ev.new_value) {
            gaugeCombo_->setCurrentIndex(ev.new_value);
        }
    }, false);

    precisionSub_ = model_.selectedPrecision.subscribe([this](const auto& ev) {
        if (precisionCombo_->currentIndex() != ev.new_value) {
            precisionCombo_->setCurrentIndex(ev.new_value);
        }
    }, false);

    pressureUnitSub_ = model_.selectedPressureUnit.subscribe([this](const auto& ev) {
        if (pressureUnitCombo_->currentIndex() != ev.new_value) {
            pressureUnitCombo_->setCurrentIndex(ev.new_value);
        }
    }, false);

    printerSub_ = model_.selectedPrinter.subscribe([this](const auto& ev) {
        if (printerCombo_->currentIndex() != ev.new_value) {
            printerCombo_->setCurrentIndex(ev.new_value);
        }
    }, false);

    displacementCombo_->setCurrentIndex(model_.selectedDisplacement.get_copy());
    gaugeCombo_->setCurrentIndex(model_.selectedGauge.get_copy());
    precisionCombo_->setCurrentIndex(model_.selectedPrecision.get_copy());
    pressureUnitCombo_->setCurrentIndex(model_.selectedPressureUnit.get_copy());
    printerCombo_->setCurrentIndex(model_.selectedPrinter.get_copy());
}
