#include "QtCircleOverlaySettingsWidget.h"

#include <QColorDialog>
#include <QFormLayout>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QString>

#include "viewmodels/settings/CircleOverlaySettingsViewModel.h"

namespace {
QColor toQColor(std::uint32_t rgba)
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

QString colorButtonStyle(const QColor& color)
{
    const QString fg = color.lightness() < 128 ? QStringLiteral("#ffffff") : QStringLiteral("#000000");
    return QStringLiteral("background-color:%1; color:%2; border:1px solid #6b7280; padding:6px 10px;")
        .arg(color.name(QColor::HexRgb), fg);
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
    auto* layout = new QFormLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    diameter_spin_ = new QSpinBox(this);
    diameter_spin_->setRange(0, 100);
    diameter_spin_->setSuffix(" %");

    color1_button_ = new QPushButton(this);
    color2_button_ = new QPushButton(this);
    color1_button_->setMinimumWidth(120);
    color2_button_->setMinimumWidth(120);

    layout->addRow(tr("Диаметр"), diameter_spin_);
    layout->addRow(tr("Цвет 1"), color1_button_);
    layout->addRow(tr("Цвет 2"), color2_button_);
}

void QtCircleOverlaySettingsWidget::connectUi()
{
    connect(diameter_spin_, qOverload<int>(&QSpinBox::valueChanged), this, [this](int value) {
        model_.circleDiameterPercent().set(value);
    });

    connect(color1_button_, &QPushButton::clicked, this, [this]() {
        const QColor current = toQColor(model_.circleColor1().get_copy());
        const QColor selected = QColorDialog::getColor(current, this, tr("Выберите цвет 1"));
        if (selected.isValid()) {
            model_.circleColor1().set(toRgba(selected));
        }
    });

    connect(color2_button_, &QPushButton::clicked, this, [this]() {
        const QColor current = toQColor(model_.circleColor2().get_copy());
        const QColor selected = QColorDialog::getColor(current, this, tr("Выберите цвет 2"));
        if (selected.isValid()) {
            model_.circleColor2().set(toRgba(selected));
        }
    });
}

void QtCircleOverlaySettingsWidget::connectViewModel()
{
    diameter_sub_ = model_.circleDiameterPercent().subscribe([this](const auto& event) {
        QSignalBlocker blocker(diameter_spin_);
        diameter_spin_->setValue(event.new_value);
    }, false);

    color1_sub_ = model_.circleColor1().subscribe([this](const auto& event) {
        updateColorButton(color1_button_, event.new_value);
    }, false);

    color2_sub_ = model_.circleColor2().subscribe([this](const auto& event) {
        updateColorButton(color2_button_, event.new_value);
    }, false);

    diameter_spin_->setValue(model_.circleDiameterPercent().get_copy());
    updateColorButton(color1_button_, model_.circleColor1().get_copy());
    updateColorButton(color2_button_, model_.circleColor2().get_copy());
}

void QtCircleOverlaySettingsWidget::updateColorButton(QPushButton* button, std::uint32_t color) const
{
    const QColor qcolor = toQColor(color);
    button->setText(qcolor.name(QColor::HexRgb).toUpper());
    button->setStyleSheet(colorButtonStyle(qcolor));
}
