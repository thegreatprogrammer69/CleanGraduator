#include "QtComponentConfigWidget.h"

#include "viewmodels/component/ComponentConfigViewModel.h"

#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QCheckBox>

#include "application/models/config_parameter/ActionConfigParameter.h"
#include "application/models/config_parameter/BoolConfigParameter.h"
#include "application/models/config_parameter/IntConfigParameter.h"
#include "application/models/config_parameter/TextConfigParameter.h"

using namespace application::config;

QtComponentConfigWidget::QtComponentConfigWidget(
    mvvm::ComponentConfigViewModel& viewModel,
    QWidget* parent
)
    : QWidget(parent)
    , viewModel_(viewModel)
{
    rootLayout_ = new QVBoxLayout(this);
    gridLayout_ = new QGridLayout();
    reloadButton_ = new QPushButton("Reload");

    rootLayout_->addLayout(gridLayout_);
    rootLayout_->addWidget(reloadButton_);

    // Подписка на schema
    viewModel_.schema.subscribe(
        [this](const auto& event) {
            buildForm(event.new_value);
        }
    );

    connect(reloadButton_, &QPushButton::clicked,
            this,
            [this]() {
                viewModel_.load();
            });

    viewModel_.load();
}



void QtComponentConfigWidget::buildForm(
    const application::models::ConfigSchema& schema)
{
    // Очистка layout
    QLayoutItem* item;
    while ((item = gridLayout_->takeAt(0)) != nullptr) {
        if (item->widget())
            delete item->widget();
        delete item;
    }

    int autoRow = 0;

    for (const auto& ptr : schema)
    {
        const application::models::ConfigParameter param = *ptr;

        QWidget* editor = createEditor(param);
        if (!editor)
            continue;

        int row = param.layout.row >= 0
                    ? param.layout.row
                    : autoRow++;

        int col = param.layout.column;

        // Action-параметры без label
        if (dynamic_cast<const application::models::ActionConfigParameter*>(&param))
        {
            gridLayout_->addWidget(
                editor,
                row,
                col,
                1,
                param.layout.columnSpan
            );
            continue;
        }

        auto* label = new QLabel(
            QString::fromStdString(param.description));

        gridLayout_->addWidget(label, row, col * 2);
        gridLayout_->addWidget(editor,
                               row,
                               col * 2 + 1,
                               1,
                               param.layout.columnSpan);
    }
}

QWidget* QtComponentConfigWidget::createEditor(
    const application::models::ConfigParameter& param)
{
    if (auto* p = dynamic_cast<const application::models::IntConfigParameter*>(&param))
    {
        auto* spin = new QSpinBox();
        spin->setRange(p->min, p->max);
        spin->setValue(p->default_value);

        connect(spin,
                qOverload<int>(&QSpinBox::valueChanged),
                this,
                [this, name = p->name](int value) {
                    viewModel_.setInt(name, value);
                });

        return spin;
    }

    if (auto* p = dynamic_cast<const application::models::BoolConfigParameter*>(&param))
    {
        auto* check = new QCheckBox();
        check->setChecked(p->default_value);

        connect(check,
                &QCheckBox::toggled,
                this,
                [this, name = p->name](bool value) {
                    viewModel_.setBool(name, value);
                });

        return check;
    }

    if (auto* p = dynamic_cast<const application::models::TextConfigParameter*>(&param))
    {
        auto* edit = new QLineEdit(
            QString::fromStdString(p->default_value));

        connect(edit,
                &QLineEdit::textChanged,
                this,
                [this, name = p->name](const QString& value) {
                    viewModel_.setText(name, value.toStdString());
                });

        return edit;
    }

    if (auto* p = dynamic_cast<const application::models::ActionConfigParameter*>(&param))
    {
        auto* button = new QPushButton(
            QString::fromStdString(p->description));

        connect(button,
                &QPushButton::clicked,
                this,
                [this, name = p->name]() {
                    viewModel_.invoke(name);
                });

        return button;
    }

    return nullptr;
}
