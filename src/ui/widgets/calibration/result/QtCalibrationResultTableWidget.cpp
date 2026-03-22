#include "QtCalibrationResultTableWidget.h"

#include <QHeaderView>
#include <QResizeEvent>
#include <QShowEvent>
#include <QMetaObject>
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
        updateGeometry();
        updateSectionSizes();
    }, Qt::QueuedConnection);
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
        QMetaObject::invokeMethod(this, [this] {
            updateGeometry();
            updateSectionSizes();
            updateSpans();
            viewport()->update();
        }, Qt::QueuedConnection);
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

void QtCalibrationResultTableWidget::showEvent(QShowEvent* event)
{
    QTableView::showEvent(event);
    updateGeometry();
    updateSectionSizes();
    updateSpans();
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
            max_width = std::max(max_width, fm.horizontalAdvance(text) + 12);
        }

        verticalHeader()->setFixedWidth(max_width);
    }

    if (horizontalHeader()->isVisible()) {
        horizontalHeader()->setFixedHeight(std::max(horizontalHeader()->minimumHeight(), 28));
    }

    // Колонки растягиваем по доступной ширине.
    const int viewport_width = viewport()->width();
    if (viewport_width > 0) {
        const int base_width = viewport_width / column_count;
        int remainder = viewport_width % column_count;

        for (int col = 0; col < column_count; ++col) {
            const int width = base_width + (remainder > 0 ? 1 : 0);
            setColumnWidth(col, width);
            if (remainder > 0) {
                --remainder;
            }
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
    updateSpans();
}

void QtCalibrationResultTableWidget::updateSpans()
{
    clearSpans();

    const auto* current_model = model();
    if (current_model == nullptr) {
        return;
    }

    const int row_count = current_model->rowCount();
    const int column_count = current_model->columnCount();
    if (row_count < 4 || column_count <= 0) {
        return;
    }

    const int total_angle_row = row_count - 4;
    const int current_angle_row = row_count - 1;

    for (int col = 0; col + 1 < column_count; col += 2) {
        setSpan(total_angle_row, col, 1, 2);
        setSpan(current_angle_row, col, 1, 2);
    }
}

} // namespace ui
