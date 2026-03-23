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

    QMetaObject::invokeMethod(this, [this] {
        refreshLayout();
    }, Qt::QueuedConnection);
}

void QtCalibrationResultTableWidget::setupUi()
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    setWordWrap(false);
    setTextElideMode(Qt::ElideNone);
    setAlternatingRowColors(false);
    setIconSize(QSize(16, 16));

    setCornerButtonEnabled(false);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setShowGrid(true);
    setGridStyle(Qt::SolidLine);

    horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    horizontalHeader()->setStretchLastSection(false);
    verticalHeader()->setStretchLastSection(false);

    horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    verticalHeader()->setDefaultAlignment(Qt::AlignCenter);
    horizontalHeader()->setHighlightSections(false);
    verticalHeader()->setHighlightSections(false);
    horizontalHeader()->setMinimumSectionSize(56);
    verticalHeader()->setMinimumSectionSize(26);

    verticalHeader()->setVisible(true);
    horizontalHeader()->setVisible(true);

    setStyleSheet(QStringLiteral(
        "QTableView {"
        "  background: #f8fafc;"
        "  alternate-background-color: #f8fafc;"
        "  gridline-color: #d6dee8;"
        "  border: 1px solid #c8d3df;"
        "  selection-background-color: #dbeafe;"
        "  selection-color: #0f172a;"
        "}"
        "QHeaderView::section {"
        "  background: #e8eef5;"
        "  color: #334155;"
        "  border: 1px solid #d6dee8;"
        "  font-weight: 600;"
        "  padding: 4px 6px;"
        "}"
    ));
}

void QtCalibrationResultTableWidget::connectModelSignals()
{
    auto refresh = [this] {
        scheduleRefreshLayout();
    };

    connect(&model_, &QAbstractItemModel::modelReset, this, refresh);
    connect(&model_, &QAbstractItemModel::layoutChanged, this, refresh);
    connect(&model_, &QAbstractItemModel::dataChanged, this, refresh);
    connect(&model_, &QAbstractItemModel::rowsInserted, this, refresh);
    connect(&model_, &QAbstractItemModel::rowsRemoved, this, refresh);
    connect(&model_, &QAbstractItemModel::columnsInserted, this, refresh);
    connect(&model_, &QAbstractItemModel::columnsRemoved, this, refresh);
    connect(verticalHeader(), &QHeaderView::sectionResized, this, [this] {
        scheduleRefreshLayout();
    });
}

void QtCalibrationResultTableWidget::resizeEvent(QResizeEvent* event)
{
    QTableView::resizeEvent(event);
    refreshLayout();
}

void QtCalibrationResultTableWidget::showEvent(QShowEvent* event)
{
    QTableView::showEvent(event);
    refreshLayout();
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

void QtCalibrationResultTableWidget::scheduleRefreshLayout()
{
    if (refresh_scheduled_) {
        return;
    }

    refresh_scheduled_ = true;
    QMetaObject::invokeMethod(this, [this] {
        refresh_scheduled_ = false;
        refreshLayout();
    }, Qt::QueuedConnection);
}

void QtCalibrationResultTableWidget::refreshLayout()
{
    updateGeometry();
    updateSectionSizes();
    viewport()->update();
}

int QtCalibrationResultTableWidget::availableColumnWidth() const
{
    int availableWidth = width() - frameWidth() * 2;

    if (verticalHeader()->isVisible()) {
        availableWidth -= verticalHeader()->width();
    }

    if (verticalScrollBar()->isVisible()) {
        availableWidth -= verticalScrollBar()->width();
    }

    return std::max(0, availableWidth);
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
        int max_width = verticalHeader()->minimumSectionSize();

        const QFontMetrics fm(verticalHeader()->font());
        for (int row = 0; row < row_count; ++row) {
            const QString text =
                model()->headerData(row, Qt::Vertical, Qt::DisplayRole).toString();
            max_width = std::max(max_width, fm.horizontalAdvance(text) + 18);
        }

        verticalHeader()->setFixedWidth(max_width);
    }

    if (horizontalHeader()->isVisible()) {
        horizontalHeader()->setFixedHeight(std::max(horizontalHeader()->minimumHeight(), 30));
    }

    const int table_width = availableColumnWidth();
    if (table_width > 0) {
        const int base_width = table_width / column_count;
        int remainder = table_width % column_count;

        for (int col = 0; col < column_count; ++col) {
            const int width = base_width + (remainder > 0 ? 1 : 0);
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

    for (int row = 0; row < row_count; ++row) {
        int h = sizeHintForRow(row);
        if (h <= 0) {
            h = verticalHeader()->defaultSectionSize();
        }
        setRowHeight(row, std::max(h, verticalHeader()->minimumSectionSize()));
    }

    updateGeometry();
}

} // namespace ui
