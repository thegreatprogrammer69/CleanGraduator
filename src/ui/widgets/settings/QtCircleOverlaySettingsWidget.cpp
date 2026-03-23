#include "QtCircleOverlaySettingsWidget.h"

#include <QColor>
#include <QColorDialog>
#include <QFormLayout>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>

#include "viewmodels/settings/CircleOverlaySettingsViewModel.h"

namespace {
QColor toQColor(std::uint32_t rgba)
{
    return QColor(
        static_cast<int>((rgba >> 24) & 0xFF),
        static_cast<int>((rgba >> 16) & 0xFF),
        static_cast<int>((rgba >> 8) & 0xFF),
        static_cast<int>(rgba & 0xFF)
    );
}

std::uint32_t toRgba(const QColor& color)
{
    return (static_cast<std::uint32_t>(color.red()) << 24)
        | (static_cast<std::uint32_t>(color.green()) << 16)
        | (static_cast<std::uint32_t>(color.blue()) << 8)
        | static_cast<std::uint32_t>(color.alpha());
}

QString toHex(std::uint32_t rgba)
{
    return QString("#%1").arg(rgba, 8, 16, QLatin1Char('0')).toUpper();
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
    layout->setHorizontalSpacing(12);
    layout->setVerticalSpacing(8);

    diameterSpinBox_ = new QSpinBox(this);
    diameterSpinBox_->setRange(0, 100);
    diameterSpinBox_->setSuffix(" %");

    color1Button_ = new QPushButton(this);
    color2Button_ = new QPushButton(this);

    layout->addRow(tr("Диаметр"), diameterSpinBox_);
    layout->addRow(tr("Цвет 1"), color1Button_);
    layout->addRow(tr("Цвет 2"), color2Button_);
}

void QtCircleOverlaySettingsWidget::connectUi()
{
    connect(diameterSpinBox_, qOverload<int>(&QSpinBox::valueChanged),
            this,
            [this](int value) {
                model_.setDiameterPercent(value);
            });

    connect(color1Button_, &QPushButton::clicked,
            this,
            [this]() {
                const QColor current = toQColor(model_.color1.get_copy());
                const QColor selected = QColorDialog::getColor(current, this, tr("Выберите цвет 1"), QColorDialog::ShowAlphaChannel);
                if (selected.isValid()) {
                    model_.setColor1(toRgba(selected));
                }
            });

    connect(color2Button_, &QPushButton::clicked,
            this,
            [this]() {
                const QColor current = toQColor(model_.color2.get_copy());
                const QColor selected = QColorDialog::getColor(current, this, tr("Выберите цвет 2"), QColorDialog::ShowAlphaChannel);
                if (selected.isValid()) {
                    model_.setColor2(toRgba(selected));
                }
            });
}

void QtCircleOverlaySettingsWidget::connectViewModel()
{
    diameterSub_ = model_.diameterPercent.subscribe([this](const auto& event) {
        const QSignalBlocker blocker(diameterSpinBox_);
        diameterSpinBox_->setValue(event.new_value);
    }, false);

    color1Sub_ = model_.color1.subscribe([this](const auto& event) {
        updateColorButton(color1Button_, event.new_value);
    }, false);

    color2Sub_ = model_.color2.subscribe([this](const auto& event) {
        updateColorButton(color2Button_, event.new_value);
    }, false);

    updateColorButton(color1Button_, model_.color1.get_copy());
    updateColorButton(color2Button_, model_.color2.get_copy());
    diameterSpinBox_->setValue(model_.diameterPercent.get_copy());
}

void QtCircleOverlaySettingsWidget::updateColorButton(QPushButton* button, std::uint32_t rgba)
{
    if (!button) {
        return;
    }

    const QColor color = toQColor(rgba);
    button->setText(toHex(rgba));

    const QColor textColor = color.lightnessF() < 0.5 ? QColor(Qt::white) : QColor(Qt::black);
    button->setStyleSheet(QString(
        "QPushButton { background-color: rgba(%1, %2, %3, %4); color: %5; min-width: 120px; }"
    ).arg(color.red())
     .arg(color.green())
     .arg(color.blue())
     .arg(color.alpha())
     .arg(textColor.name()));
}
