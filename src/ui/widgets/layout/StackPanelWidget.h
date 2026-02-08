#ifndef CLEANGRADUATOR_STACKPANELWIDGET_H
#define CLEANGRADUATOR_STACKPANELWIDGET_H

#include <QWidget>
#include <QVector>

class QFrame;
class QHBoxLayout;
class QStackedWidget;
class QPushButton;

class StackPanelWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StackPanelWidget(QWidget *parent = nullptr);

    // --- QTabWidget-like API ---
    int addTab(QWidget *page, const QString &label);
    int insertTab(int index, QWidget *page, const QString &label);
    void removeTab(int index);

    int count() const;
    int currentIndex() const;
    QWidget* currentWidget() const;

    QWidget* widget(int index) const;
    int indexOf(QWidget *page) const;

    void setCurrentIndex(int index);
    void setCurrentWidget(QWidget *page);

    QString tabText(int index) const;
    void setTabText(int index, const QString &text);

    // Access to internals if needed
    QFrame* tabsBar() const;
    QStackedWidget* stackedWidget() const;
    QPushButton* tabButton(int index) const;

    signals:
        void currentChanged(int index);

private:
    void setActiveButton(int index);
    void repolish(QWidget *w);

    QFrame        *m_tabsBar = nullptr;
    QHBoxLayout   *m_tabsLayout = nullptr;
    QStackedWidget*m_stack = nullptr;

    QVector<QPushButton*> m_buttons;
};

#endif //CLEANGRADUATOR_STACKPANELWIDGET_H