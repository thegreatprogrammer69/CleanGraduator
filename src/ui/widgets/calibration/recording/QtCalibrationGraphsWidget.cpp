#include "QtCalibrationGraphsWidget.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPainter>
#include <QPaintEvent>
#include <QPen>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_set>

namespace {

struct AxisBounds {
    double min_x = 0.0;
    double max_x = 1.0;
    double min_y = 0.0;
    double max_y = 1.0;
};

AxisBounds calcBounds(const std::vector<ui::QtCalibrationGraphsWidget::PlotSeries>& series)
{
    AxisBounds b{};
    bool has = false;

    for (const auto& s : series) {
        for (const auto& p : s.points) {
            if (!has) {
                b.min_x = b.max_x = p.x;
                b.min_y = b.max_y = p.y;
                has = true;
                continue;
            }

            b.min_x = std::min(b.min_x, p.x);
            b.max_x = std::max(b.max_x, p.x);
            b.min_y = std::min(b.min_y, p.y);
            b.max_y = std::max(b.max_y, p.y);
        }
    }

    if (!has) {
        return b;
    }

    if (std::abs(b.max_x - b.min_x) < 1e-9) {
        b.max_x += 1.0;
    }

    if (std::abs(b.max_y - b.min_y) < 1e-9) {
        b.max_y += 1.0;
    }

    return b;
}

} // namespace

namespace ui {

class QtCalibrationGraphsWidget::PlotCanvas final : public QWidget
{
public:
    explicit PlotCanvas(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        setMinimumHeight(260);
    }

    void setPlot(std::vector<PlotSeries> series, QString x_axis, QString y_axis)
    {
        series_ = std::move(series);
        x_axis_ = std::move(x_axis);
        y_axis_ = std::move(y_axis);
        update();
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        Q_UNUSED(event);

        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.fillRect(rect(), palette().base());

        const QRect plot_rect = rect().adjusted(56, 16, -16, -44);
        if (plot_rect.width() <= 8 || plot_rect.height() <= 8) {
            return;
        }

        p.setPen(QPen(QColor(150, 150, 150), 1.0));
        p.drawRect(plot_rect);

        p.setPen(QPen(QColor(95, 95, 95), 1.0, Qt::DashLine));
        for (int i = 1; i <= 4; ++i) {
            const int x = plot_rect.left() + (plot_rect.width() * i) / 5;
            const int y = plot_rect.top() + (plot_rect.height() * i) / 5;
            p.drawLine(x, plot_rect.top(), x, plot_rect.bottom());
            p.drawLine(plot_rect.left(), y, plot_rect.right(), y);
        }

        const auto bounds = calcBounds(series_);

        const auto toPixel = [&](const PlotPoint& pt) {
            const double tx = (pt.x - bounds.min_x) / (bounds.max_x - bounds.min_x);
            const double ty = (pt.y - bounds.min_y) / (bounds.max_y - bounds.min_y);

            return QPointF(
                plot_rect.left() + tx * plot_rect.width(),
                plot_rect.bottom() - ty * plot_rect.height());
        };

        for (const auto& s : series_) {
            if (s.points.empty()) {
                continue;
            }

            p.setPen(QPen(s.color, 2.0));
            QPointF prev = toPixel(s.points.front());
            p.drawEllipse(prev, 2.0, 2.0);
            for (std::size_t i = 1; i < s.points.size(); ++i) {
                const QPointF cur = toPixel(s.points[i]);
                p.drawLine(prev, cur);
                p.drawEllipse(cur, 2.0, 2.0);
                prev = cur;
            }
        }

        p.setPen(palette().windowText().color());
        p.drawText(QRect(plot_rect.left(), plot_rect.bottom() + 6, plot_rect.width(), 18),
                   Qt::AlignHCenter | Qt::AlignVCenter,
                   x_axis_);

        p.save();
        p.translate(14, plot_rect.top() + plot_rect.height() / 2);
        p.rotate(-90);
        p.drawText(QRect(-plot_rect.height() / 2, -12, plot_rect.height(), 20),
                   Qt::AlignHCenter | Qt::AlignVCenter,
                   y_axis_);
        p.restore();

        p.drawText(QRect(6, plot_rect.bottom() - 10, 46, 16),
                   Qt::AlignRight | Qt::AlignVCenter,
                   QString::number(bounds.min_y, 'f', 2));
        p.drawText(QRect(6, plot_rect.top() - 8, 46, 16),
                   Qt::AlignRight | Qt::AlignVCenter,
                   QString::number(bounds.max_y, 'f', 2));

        p.drawText(QRect(plot_rect.left() - 8, plot_rect.bottom() + 20, 70, 16),
                   Qt::AlignLeft | Qt::AlignVCenter,
                   QString::number(bounds.min_x, 'f', 2));
        p.drawText(QRect(plot_rect.right() - 60, plot_rect.bottom() + 20, 70, 16),
                   Qt::AlignRight | Qt::AlignVCenter,
                   QString::number(bounds.max_x, 'f', 2));
    }

private:
    std::vector<PlotSeries> series_;
    QString x_axis_;
    QString y_axis_;
};

QtCalibrationGraphsWidget::QtCalibrationGraphsWidget(mvvm::CalibrationSeriesViewModel& vm, QWidget* parent)
    : QWidget(parent)
    , vm_(vm)
{
    buildUi();
    rebuildAngleSources();
    refreshPlot();

    current_pressure_sub_ = vm_.current_pressure.subscribe([this](const auto&) {
        QMetaObject::invokeMethod(this, [this] { refreshPlot(); }, Qt::QueuedConnection);
    });

    current_angle_sub_ = vm_.current_angle.subscribe([this](const auto&) {
        QMetaObject::invokeMethod(this, [this] { refreshPlot(); }, Qt::QueuedConnection);
    });

    source_ids_sub_ = vm_.source_ids.subscribe([this](const auto&) {
        QMetaObject::invokeMethod(this, [this] {
            rebuildAngleSources();
            refreshPlot();
        }, Qt::QueuedConnection);
    });
}

QtCalibrationGraphsWidget::~QtCalibrationGraphsWidget() = default;

void QtCalibrationGraphsWidget::buildUi()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(8);

    auto* controls = new QHBoxLayout();
    controls->setSpacing(8);
    root->addLayout(controls);

    controls->addWidget(new QLabel(tr("График:"), this));

    graph_type_ = new QComboBox(this);
    graph_type_->addItem(tr("Давление / Время"));
    graph_type_->addItem(tr("Угол / Время"));
    graph_type_->addItem(tr("Угол / Давление"));
    controls->addWidget(graph_type_, 1);

    controls->addWidget(new QLabel(tr("Источники угла:"), this));

    angle_sources_ = new QListWidget(this);
    angle_sources_->setSelectionMode(QAbstractItemView::NoSelection);
    angle_sources_->setMaximumHeight(90);
    angle_sources_->setMinimumWidth(220);
    controls->addWidget(angle_sources_, 2);

    canvas_ = new PlotCanvas(this);
    root->addWidget(canvas_, 1);

    connect(graph_type_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &QtCalibrationGraphsWidget::onGraphTypeChanged);

    connect(angle_sources_, &QListWidget::itemChanged,
            this, &QtCalibrationGraphsWidget::onAngleSourceItemChanged);
}

void QtCalibrationGraphsWidget::onGraphTypeChanged(int index)
{
    const auto type = static_cast<GraphType>(index);
    angle_sources_->setEnabled(type != GraphType::PressureTime);
    refreshPlot();
}

void QtCalibrationGraphsWidget::onAngleSourceItemChanged(QListWidgetItem* item)
{
    Q_UNUSED(item);
    refreshPlot();
}

std::vector<int> QtCalibrationGraphsWidget::selectedAngleSourceIds() const
{
    std::vector<int> result;
    for (int i = 0; i < angle_sources_->count(); ++i) {
        auto* item = angle_sources_->item(i);
        if (!item || item->checkState() != Qt::Checked) {
            continue;
        }

        result.push_back(item->data(Qt::UserRole).toInt());
    }

    return result;
}

QColor QtCalibrationGraphsWidget::colorForIndex(int idx)
{
    static const QColor palette[] = {
        QColor(38, 139, 210),
        QColor(211, 54, 130),
        QColor(42, 161, 152),
        QColor(181, 137, 0),
        QColor(220, 50, 47),
        QColor(108, 113, 196)
    };

    return palette[idx % (sizeof(palette) / sizeof(palette[0]))];
}

void QtCalibrationGraphsWidget::rebuildAngleSources()
{
    std::unordered_set<int> checked;
    for (int i = 0; i < angle_sources_->count(); ++i) {
        auto* item = angle_sources_->item(i);
        if (item && item->checkState() == Qt::Checked) {
            checked.insert(item->data(Qt::UserRole).toInt());
        }
    }

    std::vector<int> ids;
    ids.reserve(vm_.openedSources().size());
    for (const auto& sid : vm_.openedSources()) {
        ids.push_back(sid.value);
    }

    std::sort(ids.begin(), ids.end());
    ids.erase(std::unique(ids.begin(), ids.end()), ids.end());

    angle_sources_->blockSignals(true);
    angle_sources_->clear();

    for (int id : ids) {
        auto* item = new QListWidgetItem(tr("Камера %1").arg(id), angle_sources_);
        item->setData(Qt::UserRole, id);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);

        const bool is_checked = checked.find(id) != checked.end() || checked.empty();
        item->setCheckState(is_checked ? Qt::Checked : Qt::Unchecked);
    }

    angle_sources_->blockSignals(false);
}

void QtCalibrationGraphsWidget::refreshPlot()
{
    const auto type = static_cast<GraphType>(graph_type_->currentIndex());
    std::vector<PlotSeries> series;

    if (type == GraphType::PressureTime) {
        PlotSeries pressure;
        pressure.name = tr("Давление");
        pressure.color = colorForIndex(0);

        for (const auto& e : vm_.pressureHistory()) {
            pressure.points.push_back(PlotPoint{e.time, e.pressure});
        }

        series.push_back(std::move(pressure));
        canvas_->setPlot(std::move(series), tr("Время"), tr("Давление"));
        return;
    }

    const auto selected_ids = selectedAngleSourceIds();

    if (type == GraphType::AngleTime) {
        int color_idx = 0;
        for (int id : selected_ids) {
            const auto& history = vm_.angleHistory(domain::common::SourceId{id});

            PlotSeries angle;
            angle.name = tr("Камера %1").arg(id);
            angle.color = colorForIndex(color_idx++);

            for (const auto& e : history) {
                angle.points.push_back(PlotPoint{e.time, e.angle});
            }

            series.push_back(std::move(angle));
        }

        canvas_->setPlot(std::move(series), tr("Время"), tr("Угол"));
        return;
    }

    int color_idx = 0;
    const auto& pressure = vm_.pressureHistory();

    for (int id : selected_ids) {
        const auto& history = vm_.angleHistory(domain::common::SourceId{id});

        PlotSeries angle_by_pressure;
        angle_by_pressure.name = tr("Камера %1").arg(id);
        angle_by_pressure.color = colorForIndex(color_idx++);

        for (const auto& angle : history) {
            if (pressure.empty()) {
                break;
            }

            auto it = std::min_element(
                pressure.begin(),
                pressure.end(),
                [&angle](const auto& a, const auto& b) {
                    return std::abs(a.time - angle.time) < std::abs(b.time - angle.time);
                });

            angle_by_pressure.points.push_back(PlotPoint{it->pressure, angle.angle});
        }

        series.push_back(std::move(angle_by_pressure));
    }

    canvas_->setPlot(std::move(series), tr("Давление"), tr("Угол"));
}

} // namespace ui
