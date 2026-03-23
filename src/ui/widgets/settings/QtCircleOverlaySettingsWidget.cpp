#include "QtCircleOverlaySettingsWidget.h"

#include <QColor>
#include <QColorDialog>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QPushButton>

#include "viewmodels/settings/CircleOverlaySettingsViewModel.h"

namespace {
QColor fromRgba(std::uint32_t rgba)
{
    return QColor(
        static_cast<int>((rgba >> 24) & 0xFFu),
        static_cast<int>((rgba >> 16) & 0xFFu),
        static_cast<int>((rgba >> 8) & 0xFFu),
        static_cast<int>(rgba & 0xFFu));
}

std::uint32_t toRgba(const QColor& color)
{
    return (static_cast<std::uint32_t>(color.red()) << 24)
         | (static_cast<std::uint32_t>(color.green()) << 16)
         | (static_cast<std::uint32_t>(color.blue()) << 8)
         | static_cast<std::uint32_t>(color.alpha());
}

QString buttonStyle(const QColor& color)
{
    const QColor border = color.lightness() > 128 ? QColor("#4b5563") : QColor("#d1d5db");
    return QString(
        "background-color: rgba(%1, %2, %3, %4); border: 1px solid %5; border-radius: 6px; min-height: 28px;")
        .arg(color.red())
        .arg(color.green())
        .arg(color.blue())
        .arg(color.alpha())
        .arg(border.name());
}
}

QtCircleOverlaySettingsWidget::QtCircleOverlaySettingsWidget(
    mvvm::CircleOverlaySettingsViewModel& model,
    QWidget* parent)
    : QWidget(parent)
    , model_(model)
{
    buildUi();
    connectUi();
    connectViewModel();
}

QtCircleOverlaySettingsWidget::~QtCircleOverlaySettingsWidget() = default;

void QtCircleOverlaySettingsWidget::buildUi()
{
    auto* rootLayout = new QFormLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(8);

    diameterSpin_ = new QDoubleSpinBox(this);
    diameterSpin_->setRange(0.0, 100.0);
    diameterSpin_->setDecimals(1);
    diameterSpin_->setSingleStep(1.0);
    diameterSpin_->setSuffix(" %");

    auto* colorsLayout = new QHBoxLayout();
    colorsLayout->setContentsMargins(0, 0, 0, 0);
    colorsLayout->setSpacing(8);

    color1Button_ = new QPushButton(tr("Цвет 1"), this);
    color2Button_ = new QPushButton(tr("Цвет 2"), this);

    colorsLayout->addWidget(color1Button_);
    colorsLayout->addWidget(color2Button_);
    colorsLayout->addStretch();

    rootLayout->addRow(tr("Диаметр"), diameterSpin_);
    rootLayout->addRow(tr("Цвета"), colorsLayout);
}

void QtCircleOverlaySettingsWidget::connectUi()
{
    connect(
        diameterSpin_,
        qOverload<double>(&QDoubleSpinBox::valueChanged),
        this,
        [this](double value) {
            model_.setDiameterPercent(static_cast<float>(value));
        });

    connect(color1Button_, &QPushButton::clicked, this, &QtCircleOverlaySettingsWidget::chooseColor1);
    connect(color2Button_, &QPushButton::clicked, this, &QtCircleOverlaySettingsWidget::chooseColor2);
}

void QtCircleOverlaySettingsWidget::connectViewModel()
{
    diameter_sub_ = model_.diameter_percent.subscribe([this](const auto& event) {
        const double value = static_cast<double>(event.new_value);
        if (!qFuzzyCompare(diameterSpin_->value() + 1.0, value + 1.0)) {
            diameterSpin_->setValue(value);
        }
    });

    color1_sub_ = model_.color1.subscribe([this](const auto& event) {
        updateButtonColor(color1Button_, event.new_value);
    });

    color2_sub_ = model_.color2.subscribe([this](const auto& event) {
        updateButtonColor(color2Button_, event.new_value);
    });

    diameterSpin_->setValue(static_cast<double>(model_.diameter_percent.get_copy()));
    updateButtonColor(color1Button_, model_.color1.get_copy());
    updateButtonColor(color2Button_, model_.color2.get_copy());
}

void QtCircleOverlaySettingsWidget::updateButtonColor(QPushButton* button, std::uint32_t rgba)
{
    if (button == nullptr) {
        return;
    }

    const QColor color = fromRgba(rgba);
    button->setStyleSheet(buttonStyle(color));
    button->setText(color.name(QColor::HexRgb).toUpper());
}

void QtCircleOverlaySettingsWidget::chooseColor1()
{
    const QColor current = fromRgba(model_.color1.get_copy());
    const QColor selected = QColorDialog::getColor(current, this, tr("Выберите цвет 1"), QColorDialog::ShowAlphaChannel);
    if (selected.isValid()) {
        model_.setColor1(toRgba(selected));
    }
}

void QtCircleOverlaySettingsWidget::chooseColor2()
{
    const QColor current = fromRgba(model_.color2.get_copy());
    const QColor selected = QColorDialog::getColor(current, this, tr("Выберите цвет 2"), QColorDialog::ShowAlphaChannel);
    if (selected.isValid()) {
        model_.setColor2(toRgba(selected));
    }
}
