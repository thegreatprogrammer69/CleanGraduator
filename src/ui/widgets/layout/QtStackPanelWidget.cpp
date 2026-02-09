#include "QtStackPanelWidget.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QPushButton>
#include <QStyle>
#include <QVariant>

using namespace ui::widgets;

static QPushButton* makeTabButton(const QString &text, QWidget *parent)
{
    auto *b = new QPushButton(text, parent);
    b->setObjectName("tabBtn");          // как у тебя
    b->setCheckable(true);
    b->setCursor(Qt::PointingHandCursor);
    b->setMinimumHeight(32);
    return b;
}

QtStackPanelWidget::QtStackPanelWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(8);

    m_tabsBar = new QFrame(this);
    m_tabsBar->setObjectName("tabsBar");
    m_tabsBar->setAttribute(Qt::WA_StyledBackground, true);

    m_tabsLayout = new QHBoxLayout(m_tabsBar);
    m_tabsLayout->setContentsMargins(0, 0, 0, 0);
    m_tabsLayout->setSpacing(0);
    m_tabsLayout->addStretch(1); // держим stretch последним

    m_stack = new QStackedWidget(this);
    m_stack->setFrameShape(QFrame::NoFrame);

    root->addWidget(m_tabsBar);
    root->addWidget(m_stack, 1);

    connect(m_stack, &QStackedWidget::currentChanged, this, [this](int idx){
        setActiveButton(idx);
        emit currentChanged(idx);
    });
}

int QtStackPanelWidget::addTab(QWidget *page, const QString &label)
{
    return insertTab(count(), page, label);
}

int QtStackPanelWidget::insertTab(int index, QWidget *page, const QString &label)
{
    if (!page) return -1;

    index = qBound(0, index, count());

    // 1) stack
    m_stack->insertWidget(index, page);

    // 2) button
    auto *btn = makeTabButton(label, m_tabsBar);
    btn->setProperty("active", false);

    // layout: insert before the ending stretch
    m_tabsLayout->insertWidget(index, btn);
    m_buttons.insert(index, btn);

    connect(btn, &QPushButton::clicked, this, [this, btn]{
        const int idx = m_buttons.indexOf(btn);
        if (idx >= 0) setCurrentIndex(idx);
    });

    // если это первый таб — активируем
    if (count() == 1) {
        setCurrentIndex(0);
    } else {
        setActiveButton(m_stack->currentIndex());
    }

    return index;
}

void QtStackPanelWidget::removeTab(int index)
{
    if (index < 0 || index >= count()) return;

    QWidget *page = m_stack->widget(index);
    QPushButton *btn = m_buttons.value(index, nullptr);

    // remove from stack (не удаляет виджет)
    m_stack->removeWidget(page);
    if (page) page->setParent(nullptr); // ближе к поведению QTabWidget::removeTab

    // remove button
    if (btn) {
        m_tabsLayout->removeWidget(btn);
        m_buttons.removeAt(index);
        btn->deleteLater();
    }

    // корректируем текущий индекс
    if (count() == 0) {
        setActiveButton(-1);
        return;
    }
    const int newIndex = qMin(index, count() - 1);
    setCurrentIndex(newIndex);
}

int QtStackPanelWidget::count() const
{
    return m_stack->count();
}

int QtStackPanelWidget::currentIndex() const
{
    return m_stack->currentIndex();
}

QWidget* QtStackPanelWidget::currentWidget() const
{
    return m_stack->currentWidget();
}

QWidget* QtStackPanelWidget::widget(int index) const
{
    return m_stack->widget(index);
}

int QtStackPanelWidget::indexOf(QWidget *page) const
{
    return m_stack->indexOf(page);
}

void QtStackPanelWidget::setCurrentIndex(int index)
{
    if (index < 0 || index >= count()) return;
    if (index == m_stack->currentIndex()) return;
    m_stack->setCurrentIndex(index);
    // setActiveButton вызовется через сигнал currentChanged от m_stack
}

void QtStackPanelWidget::setCurrentWidget(QWidget *page)
{
    const int idx = indexOf(page);
    if (idx >= 0) setCurrentIndex(idx);
}

QString QtStackPanelWidget::tabText(int index) const
{
    auto *btn = tabButton(index);
    return btn ? btn->text() : QString();
}

void QtStackPanelWidget::setTabText(int index, const QString &text)
{
    auto *btn = tabButton(index);
    if (!btn) return;
    btn->setText(text);
}

QFrame* QtStackPanelWidget::tabsBar() const
{
    return m_tabsBar;
}

QStackedWidget* QtStackPanelWidget::stackedWidget() const
{
    return m_stack;
}

QPushButton* QtStackPanelWidget::tabButton(int index) const
{
    if (index < 0 || index >= m_buttons.size()) return nullptr;
    return m_buttons[index];
}

void QtStackPanelWidget::setActiveButton(int index)
{
    for (int i = 0; i < m_buttons.size(); ++i) {
        const bool active = (i == index);
        auto *btn = m_buttons[i];
        btn->setChecked(active);
        btn->setProperty("active", active);
        repolish(btn);
    }
}

void QtStackPanelWidget::repolish(QWidget *w)
{
    if (!w) return;
    w->style()->unpolish(w);
    w->style()->polish(w);
    w->update();
}
