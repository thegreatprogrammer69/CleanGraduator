#include "QtCalibrationResultTableWidget.h"

#include <QHeaderView>
#include <QResizeEvent>
#include <QShowEvent>
#include <QMetaObject>
#include <QScrollBar>
#include <algorithm>

namespace ui {

QtCalibrationResultTableWidget::QtCalibrationResultTableWidget(
    mvvm::CalibrationResultTableViewModel& vm,
    QWidget* parent)
    : QTableView(parent)
    , model_({ vm }, this)
{
    QTableView::setModel(&model_);
    setupUi();
    connectModelSignals();

    requestSectionSizeUpdate();
}

void QtCalibrationResultTableWidget::setupUi()
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    setWordWrap(false);
    setTextElideMode(Qt::ElideNone);

    setCornerButtonEnabled(false);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setShowGrid(true);
    setAlternatingRowColors(false);
    setStyleSheet(QStringLiteral(
        "QTableView {"
        " background: #ffffff;"
        " alternate-background-color: #f7fafc;"
        " gridline-color: #d6dee8;"
        " border: 1px solid #d6dee8;"
        " border-radius: 4px;"
        " selection-background-color: #d7e9ff;"
        " selection-color: #102a43;"
        "}"
        "QHeaderView::section {"
        " background: #eef3f8;"
        " color: #25364a;"
        " border: 0;"
        " border-right: 1px solid #d6dee8;"
        " border-bottom: 1px solid #d6dee8;"
        " padding: 6px 8px;"
        " font-weight: 600;"
        "}"
    ));

    horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    horizontalHeader()->setStretchLastSection(false);
    verticalHeader()->setStretchLastSection(false);

    horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    verticalHeader()->setDefaultAlignment(Qt::AlignCenter);

    verticalHeader()->setVisible(true);
    horizontalHeader()->setVisible(true);
}

void QtCalibrationResultTableWidget::connectModelSignals()
{
    auto refresh = [this] {
        requestSectionSizeUpdate();
    };

    connect(&model_, &QAbstractItemModel::modelReset, this, refresh);
    connect(&model_, &QAbstractItemModel::layoutChanged, this, refresh);
    connect(&model_, &QAbstractItemModel::dataChanged, this, refresh);
    connect(&model_, &QAbstractItemModel::rowsInserted, this, refresh);
    connect(&model_, &QAbstractItemModel::rowsRemoved, this, refresh);
    connect(&model_, &QAbstractItemModel::columnsInserted, this, refresh);
    connect(&model_, &QAbstractItemModel::columnsRemoved, this, refresh);
}

void QtCalibrationResultTableWidget::resizeEvent(QResizeEvent* event)
{
    QTableView::resizeEvent(event);
    updateSectionSizes();
}

void QtCalibrationResultTableWidget::requestSectionSizeUpdate()
{
    if (section_size_update_pending_) {
        return;
    }

    section_size_update_pending_ = true;
    QMetaObject::invokeMethod(this, [this] {
        section_size_update_pending_ = false;
        updateGeometry();
        updateSectionSizes();
        viewport()->update();
    }, Qt::QueuedConnection);
}

void QtCalibrationResultTableWidget::showEvent(QShowEvent* event)
{
    QTableView::showEvent(event);
    requestSectionSizeUpdate();
}

QSize QtCalibrationResultTableWidget::sizeHint() const
{
    if (model() == nullptr) {
        return QTableView::sizeHint();
    }

    const int rows = model()->rowCount();
    const int cols = model()->columnCount();

    int totalHeight = frameWidth() * 2;
    int totalWidth = frameWidth() * 2;

    if (horizontalHeader()->isVisible()) {
        totalHeight += horizontalHeader()->height();
    }

    if (verticalHeader()->isVisible()) {
        totalWidth += verticalHeader()->width();
    }

    for (int row = 0; row < rows; ++row) {
        int h = rowHeight(row);
        if (h <= 0) {
            h = sizeHintForRow(row);
        }
        if (h <= 0) {
            h = verticalHeader()->defaultSectionSize();
        }
        totalHeight += h;
    }

    for (int col = 0; col < cols; ++col) {
        int w = columnWidth(col);
        if (w <= 0) {
            w = horizontalHeader()->defaultSectionSize();
        }
        totalWidth += w;
    }

    return { totalWidth, totalHeight };
}

QSize QtCalibrationResultTableWidget::minimumSizeHint() const
{
    return QTableView::minimumSizeHint();
}

void QtCalibrationResultTableWidget::updateSectionSizes()
{
    if (model() == nullptr) {
        return;
    }

    clearSpans();

    const int row_count = model()->rowCount();
    const int column_count = model()->columnCount();

    if (column_count <= 0) {
        return;
    }

    if (verticalHeader()->isVisible()) {
        int max_width = std::max(verticalHeader()->minimumSectionSize(), 72);

        const QFontMetrics fm(verticalHeader()->font());
        for (int row = 0; row < row_count; ++row) {
            const QString text =
                model()->headerData(row, Qt::Vertical, Qt::DisplayRole).toString();
            max_width = std::max(max_width, fm.horizontalAdvance(text) + 18);
        }

        verticalHeader()->setFixedWidth(max_width);
    }

    updateGeometries();

    if (horizontalHeader()->isVisible()) {
        horizontalHeader()->setFixedHeight(std::max(horizontalHeader()->minimumHeight(), 28));
    }

    // Колонки растягиваем по доступной ширине.
    const int available_width = std::max(0, viewport()->width() - verticalScrollBar()->sizeHint().width() * (verticalScrollBar()->isVisible() ? 1 : 0));
    if (available_width > 0) {
        const int base_width = available_width / column_count;
        int remainder = available_width % column_count;

        for (int col = 0; col < column_count; ++col) {
            const int width = std::max(base_width + (remainder > 0 ? 1 : 0), 1);
            setColumnWidth(col, width);
            if (remainder > 0) {
                --remainder;
            }
        }
    }

    for (int row = 0; row < row_count; ++row) {
        if (!model_.isPairMergedRow(row)) {
            continue;
        }

        for (int col = 0; col < column_count; col += 2) {
            setSpan(row, col, 1, 2);
        }
    }

    // Строкам задаём нормальную "естественную" высоту.
    for (int row = 0; row < row_count; ++row) {
        int h = sizeHintForRow(row);
        if (h <= 0) {
            h = verticalHeader()->defaultSectionSize();
        }
        setRowHeight(row, h);
    }

    updateGeometry();
}

} // namespace ui