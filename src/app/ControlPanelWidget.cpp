// #ifndef CONTROLPANELWIDGET_H
// #define CONTROLPANELWIDGET_H
//
// #include <QWidget>
// #include <QTabWidget>
// #include <QGroupBox>
// #include <QPushButton>
// #include <QVBoxLayout>
// #include <QApplication>
//
// class ControlPanelWidget : public QWidget
// {
// public:
//     explicit ControlPanelWidget(QWidget* parent = nullptr)
//         : QWidget(parent)
//     {
//         auto* rootLayout = new QVBoxLayout(this);
//
//         auto* tabs = new QTabWidget(this);
//         rootLayout->addWidget(tabs);
//
//         // =============================
//         // ВКЛАДКА 1 — КЛАПАНЫ
//         // =============================
//         auto* valvesTab = new QWidget;
//         auto* valvesLayout = new QVBoxLayout(valvesTab);
//
//         auto* valvesGroup = new QGroupBox("Управление клапанами");
//         auto* valvesGroupLayout = new QVBoxLayout(valvesGroup);
//
//         valvesGroupLayout->addWidget(makeButton("Открыть впускной"));
//         valvesGroupLayout->addWidget(makeButton("Открыть выпускной"));
//         valvesGroupLayout->addWidget(makeButton("Закрыть оба"));
//
//         valvesLayout->addWidget(valvesGroup);
//         valvesLayout->addStretch();
//
//         tabs->addTab(valvesTab, "Клапаны");
//
//         // =============================
//         // ВКЛАДКА 2 — ДВИГАТЕЛЬ
//         // =============================
//         auto* motorTab = new QWidget;
//         auto* motorLayout = new QVBoxLayout(motorTab);
//
//         auto* motorGroup = new QGroupBox("Управление двигателем");
//         auto* motorGroupLayout = new QVBoxLayout(motorGroup);
//
//         motorGroupLayout->addWidget(makeButton("Старт"));
//         motorGroupLayout->addWidget(makeButton("Стоп"));
//
//         motorLayout->addWidget(motorGroup);
//         motorLayout->addStretch();
//
//         tabs->addTab(motorTab, "Двигатель");
//
//         applyStyle();
//     }
//
// private:
//
//     QPushButton* makeButton(const QString& text)
//     {
//         auto* btn = new QPushButton(text);
//         btn->setMinimumHeight(36);
//         return btn;
//     }
//
//     void applyStyle()
//     {
//         setStyleSheet(R"(
//
//         /* ---------------------------
//            GROUP BOX
//         ----------------------------*/
//         QGroupBox {
//             border: 1px solid #d0d0d0;
//             border-radius: 8px;
//             margin-top: 16px;
//             padding: 12px;
//             font-weight: 600;
//         }
//
//         /* Заголовок */
//         QGroupBox::title {
//             subcontrol-origin: margin;
//             left: 12px;
//             padding: 0 4px;
//             color: #1a1a1a;
//         }
//
//         /* Hover эффект как ссылка */
//         QGroupBox::title:hover {
//             color: #0078d4;
//             text-decoration: underline;
//         }
//
//         /* ---------------------------
//            BUTTON
//         ----------------------------*/
//         QPushButton {
//             background: #f5f5f5;
//             border: 1px solid #c8c8c8;
//             border-radius: 8px;
//             padding: 6px;
//             font-size: 14px;
//         }
//
//         QPushButton:hover {
//             background: #e6f2ff;
//             border: 1px solid #3399ff;
//         }
//
//         QPushButton:pressed {
//             background: #d0e7ff;
//         }
//
//         /* ---------------------------
//            TABS
//         ----------------------------*/
//         QTabWidget::pane {
//             border: 1px solid #d0d0d0;
//             border-radius: 8px;
//             padding: 6px;
//         }
//
//         QTabBar::tab {
//             background: #eeeeee;
//             padding: 6px 12px;
//             border-radius: 6px;
//             margin-right: 4px;
//         }
//
//         QTabBar::tab:selected {
//             background: #ffffff;
//             border: 1px solid #c8c8c8;
//         }
//
//         )");
//     }
// };
//
// #endif // CONTROLPANELWIDGET_H
