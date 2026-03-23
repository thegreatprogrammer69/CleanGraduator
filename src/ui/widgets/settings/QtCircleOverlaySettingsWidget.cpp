#include "QtCircleOverlaySettingsWidget.h"

#include <algorithm>
#include <cstdint>

#include <QColor>
#include <QColorDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSlider>

#include "viewmodels/video/ICircleOverlaySettingsViewModel.h"

namespace {

std::uint32_t toRgba(const QColor& color)
{
    return (static_cast<std::uint32_t>(color.red()) << 24)
        | (static_cast<std::uint32_t>(color.green()) << 16)
        | (static_cast<std::uint32_t>(color.blue()) << 8)
        | static_cast<std::uint32_t>(color.alpha());
}

QColor fromRgba(std::uint32_t color)
{
    return QColor(
        static_cast<int>((color >> 24) & 0xFFu),
        static_cast<int>((color >> 16) & 0xFFu),
        static_cast<int>((color >> 8) & 0xFFu),
        static_cast<int>(color & 0xFFu));
}

QString sliderLabelText(int value)
{
    return QString::number(value) + "%";
}

}

QtCircleOverlaySettingsWidget::QtCircleOverlaySettingsWidget(
    mvvm::ICircleOverlaySettingsViewModel& model,
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

    auto* diameterRow = new QWidget(this);
    auto* diameterLayout = new QHBoxLayout(diameterRow);
    diameterLayout->setContentsMargins(0, 0, 0, 0);
    diameterLayout->setSpacing(8);

    diameterSlider_ = new QSlider(Qt::Horizontal, diameterRow);
    diameterSlider_->setRange(0, 100);

    diameterValueLabel_ = new QLabel(diameterRow);
    diameterValueLabel_->setMinimumWidth(48);

    diameterLayout->addWidget(diameterSlider_);
    diameterLayout->addWidget(diameterValueLabel_);

    color1Button_ = new QPushButton(tr("Выбрать цвет"), this);
    color2Button_ = new QPushButton(tr("Выбрать цвет"), this);

    layout->addRow(tr("Диаметр"), diameterRow);
    layout->addRow(tr("Цвет 1"), color1Button_);
    layout->addRow(tr("Цвет 2"), color2Button_);
}

void QtCircleOverlaySettingsWidget::connectUi()
{
    connect(diameterSlider_, &QSlider::valueChanged, this, [this](int value) {
        model_.circleDiameterPercent().set(std::clamp(value, 0, 100));
    });

    connect(color1Button_, &QPushButton::clicked, this, [this]() {
        const QColor current = fromRgba(model_.circleColor1().get_copy());
        const QColor chosen = QColorDialog::getColor(current, this, tr("Цвет 1"));
        if (chosen.isValid()) {
            model_.circleColor1().set(toRgba(chosen));
        }
    });

    connect(color2Button_, &QPushButton::clicked, this, [this]() {
        const QColor current = fromRgba(model_.circleColor2().get_copy());
        const QColor chosen = QColorDialog::getColor(current, this, tr("Цвет 2"));
        if (chosen.isValid()) {
            model_.circleColor2().set(toRgba(chosen));
        }
    });
}

void QtCircleOverlaySettingsWidget::connectViewModel()
{
    diameterSub_ = model_.circleDiameterPercent().subscribe([this](const auto& event) {
        const int value = std::clamp(event.new_value, 0, 100);
        {
            const QSignalBlocker blocker(diameterSlider_);
            if (diameterSlider_->value() != value) {
                diameterSlider_->setValue(value);
            }
        }
        diameterValueLabel_->setText(sliderLabelText(value));
    }, false);

    color1Sub_ = model_.circleColor1().subscribe([this](const auto& event) {
        applyColorButtonStyle(color1Button_, event.new_value);
    }, false);

    color2Sub_ = model_.circleColor2().subscribe([this](const auto& event) {
        applyColorButtonStyle(color2Button_, event.new_value);
    }, false);

    diameterValueLabel_->setText(sliderLabelText(model_.circleDiameterPercent().get_copy()));
    diameterSlider_->setValue(model_.circleDiameterPercent().get_copy());
    applyColorButtonStyle(color1Button_, model_.circleColor1().get_copy());
    applyColorButtonStyle(color2Button_, model_.circleColor2().get_copy());
}

void QtCircleOverlaySettingsWidget::applyColorButtonStyle(QPushButton* button, std::uint32_t color)
{
    const QColor qcolor = fromRgba(color);
    const QColor textColor = qcolor.lightness() < 128 ? QColor(Qt::white) : QColor(Qt::black);

    button->setText(qcolor.name(QColor::HexArgb).toUpper());
    button->setStyleSheet(
        QString("background-color: %1; color: %2;")
            .arg(qcolor.name(QColor::HexArgb), textColor.name()));
}
